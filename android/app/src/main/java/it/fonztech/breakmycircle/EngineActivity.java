package it.fonztech.breakmycircle;

import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;

import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.FullScreenContentCallback;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.OnUserEarnedRewardListener;
import com.google.android.gms.ads.RequestConfiguration;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;
import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;
import com.google.android.gms.ads.rewarded.RewardItem;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.messaging.FirebaseMessaging;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;

public abstract class EngineActivity extends NativeActivity implements OnInitializationCompleteListener, OnUserEarnedRewardListener, OnCompleteListener<String> {
    private static final String TAG = EngineActivity.class.getSimpleName();

    protected static final String ASSET_PREFERENCES = "ASSET_PREFERENCES";
    protected static final String CONFIG_PREFERENCES = "CONFIG_PREFERENCES";
    protected static final String SAVE_FILE = "SAVE_FILE.json";
    protected static final String ADS_NOT_AVAILABLE_TYPE = "_not_available";

    protected static final String INTERSTITIAL_AD_DEV = "ca-app-pub-3940256099942544/1033173712";
    protected static final String REWARDED_AD_DEV = "ca-app-pub-3940256099942544/5224354917";

    protected static final String INTERSTITIAL_AD_PROD = "ca-app-pub-3837498848455030/3033316978";
    protected static final String REWARDED_AD_PROD = "ca-app-pub-3837498848455030/9074630079";

    protected static final String GAME_GP_AMOUNT = "game_powerup_amount";
    protected static final String GAME_GP_EXPIRE = "game_powerup_expire";
    protected static final String GAME_PLAY_AD_THRESHOLD = "play_ad_threshold";

    protected final InterstitialAdLoadCallback interstitialAdLoadCallback = new InterstitialAdLoadCallback() {
        @Override
        public final void onAdLoaded(@NonNull final InterstitialAd interstitialAd) {
            interstitialAd.setFullScreenContentCallback(fullScreenContentCallback);
            interstitialAd.show(EngineActivity.this);
        }

        @Override
        public final void onAdFailedToLoad(@NonNull final LoadAdError loadAdError) {
            // showError(loadAdError.getMessage());
        }
    };

    protected final RewardedAdLoadCallback rewardedAdLoadCallback = new RewardedAdLoadCallback() {
        @Override
        public void onAdFailedToLoad(@NonNull final LoadAdError loadAdError) {
            setRewardedInfo(null, 0);
            showError(loadAdError.getMessage());
        }

        @Override
        public void onAdLoaded(@NonNull final RewardedAd rewardedAd) {
            rewardedAd.show(EngineActivity.this, EngineActivity.this);
        }
    };

    protected boolean mCanShowAds;
    protected int mPlayAdThreshold;

    protected final FullScreenContentCallback fullScreenContentCallback = new FullScreenContentCallback() {
        @Override
        public final void onAdDismissedFullScreenContent() {
            setRewardedInfo(null, 0);
        }

        @Override
        public final void onAdFailedToShowFullScreenContent(@SuppressWarnings("NullableProblems") final AdError adError) {
            setRewardedInfo(null, 0);
            // showError(adError.getMessage());
        }

        @Override
        public final void onAdShowedFullScreenContent() {
        }
    };

    public static boolean IS_ACTIVE = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        IS_ACTIVE = true;

        mCanShowAds = true;

        // Initialize Firebase Messaging
        FirebaseMessaging.getInstance().getToken().addOnCompleteListener(this);

        // Initialize AdMob
        {
            final RequestConfiguration requestConfiguration = MobileAds.getRequestConfiguration()
                    .toBuilder()
                    .setTagForChildDirectedTreatment(RequestConfiguration.TAG_FOR_CHILD_DIRECTED_TREATMENT_TRUE)
                    .build();
            MobileAds.setRequestConfiguration(requestConfiguration);
            MobileAds.initialize(this, this);
        }

        // Display density
        {
            final float density = Math.max(1.0f, getResources().getDisplayMetrics().density);
            getIntent().putExtra("density", Float.toString(density));
        }

        // Canvas vertical height
        {
            final int height = statusBarHeight();
            getIntent().putExtra("canvas_vertical_height", Integer.toString(height));
        }

        // Play ad threshold
        {
            getIntent().putExtra("play_ad_threshold", "3");
        }

        // Base asset directory
        {
            String assetDir = getFilesDir().getAbsolutePath();
            if (!assetDir.endsWith(File.separator)) {
                assetDir += File.separator;
            }
            getIntent().putExtra("asset_dir", assetDir);
        }

        // Save file
        {
            final File saveFile = new File(getFilesDir(), SAVE_FILE);
            if (saveFile.exists()) {
                getIntent().putExtra("save_file", saveFile.getAbsolutePath());
            }
            else {
                try {
                    //noinspection ResultOfMethodCallIgnored
                    saveFile.createNewFile();
                    getIntent().putExtra("save_file", saveFile.getAbsolutePath());
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        // Copy all packed assets into internal app storage
        try {
            final SharedPreferences prefs = getSharedPreferences(ASSET_PREFERENCES, Context.MODE_PRIVATE);
            final SharedPreferences.Editor editor = prefs.edit();

            final JSONObject json = new JSONObject(getAssetsJson());

            final Iterator<String> iterator = json.keys();
            while (iterator.hasNext()) {
                final String fname = iterator.next();
                final JSONObject assetJson = json.getJSONObject(fname);
                final File dest = new File(getFilesDir(), fname);
                final long assetVersion = assetJson.getLong("version");

                if (prefs.getLong(fname, 0L) >= assetVersion) {
                    Log.d(TAG, "Asset " + fname + " (" + assetVersion + ") already exists in " + dest.getAbsolutePath());
                }
                else {
                    Log.d(TAG, "Copying asset " + fname + " (" + assetVersion + ") into " + dest.getAbsolutePath());

                    {
                        final File parentDir = dest.getParentFile();
                        //noinspection ConstantConditions
                        if (!parentDir.exists() && !parentDir.mkdirs()){
                            throw new RuntimeException("Could not make parent directories for " + dest.getAbsolutePath());
                        }
                    }

                    final InputStream is = getAssets().open(fname);
                    final FileOutputStream fos = new FileOutputStream(dest);

                    final byte[] buffer = new byte[1024 * 10];
                    for (int length; (length = is.read(buffer)) != -1; ) {
                        fos.write(buffer, 0, length);
                    }

                    fos.close();
                    is.close();

                    editor.putLong(fname, assetVersion);
                }
            }

            editor.apply();
        }
        catch (Exception e) {
            throw new RuntimeException(e.getMessage() != null ? e.getMessage() : "Unknown exception error");
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        IS_ACTIVE = false;
    }

    @Override
    public final void onInitializationComplete(@SuppressWarnings("NullableProblems") final InitializationStatus initializationStatus)
    {
    }

    @Override
    public final void onUserEarnedReward(@NonNull final RewardItem rewardItem)
    {
        final int rewardAmount = rewardItem.getAmount();
        final String rewardType = rewardItem.getType();
        setRewardedInfo(rewardType, rewardAmount);
    }

    @Override
    public final void onComplete(@NonNull Task<String> task) {
        if (!task.isSuccessful()) {
            Log.w(TAG, "Fetching FCM registration token failed", task.getException());
            return;
        }

        // Get new FCM registration token
        final String token = task.getResult();
        new TokenSender(token).start();
    }

    protected abstract void setRewardedInfo(final String type, final int amount);

    protected abstract String getBackendUrl();

    protected final String getAssetsJson() throws IOException {
        final InputStream is = getResources().openRawResource(R.raw.assets);
        final ByteArrayOutputStream baos = new ByteArrayOutputStream();

        final byte[] buffer = new byte[1024 * 10];
        for (int length; (length = is.read(buffer)) != -1; ) {
            baos.write(buffer, 0, length);
        }

        baos.close();
        is.close();

        return new String(baos.toByteArray());
    }

    protected final void openUrl(final String url) {
        final Intent i = new Intent(Intent.ACTION_VIEW);
        i.setData(Uri.parse(url));
        startActivity(i);
    }

    @SuppressWarnings("unused")
    protected final void showInterstitial() {
        runOnUiThread(() -> {
            if (mCanShowAds) {
                final AdRequest adRequest = new AdRequest.Builder().build();
                InterstitialAd.load(EngineActivity.this, Utility.DEBUG ? INTERSTITIAL_AD_DEV : INTERSTITIAL_AD_PROD, adRequest, interstitialAdLoadCallback);
            }
            else {
                setRewardedInfo(ADS_NOT_AVAILABLE_TYPE, 0);
            }
        });
    }

    @SuppressWarnings("unused")
    protected final void watchAdForPowerup() {
        runOnUiThread(() -> {
            if (mCanShowAds) {
                final AdRequest adRequest = new AdRequest.Builder().build();
                RewardedAd.load(EngineActivity.this, Utility.DEBUG ? REWARDED_AD_DEV : REWARDED_AD_PROD, adRequest, rewardedAdLoadCallback);
            }
            else {
                setRewardedInfo(ADS_NOT_AVAILABLE_TYPE, 0);
                new AlertDialog.Builder(EngineActivity.this)
                        .setTitle(R.string.app_name)
                        .setMessage(R.string.ad_not_available)
                        .setPositiveButton(android.R.string.ok, null)
                        .show();
            }
        });
    }

    protected final int statusBarHeight() {
        int result = 0;
        int resourceId = getResources().getIdentifier("status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            result = getResources().getDimensionPixelSize(resourceId);
        }
        return result;
    }

    protected final void showError(final String message) {
        new AlertDialog.Builder(EngineActivity.this)
                .setTitle(R.string.app_name)
                .setMessage(R.string.ad_error)
                .setPositiveButton(android.R.string.ok, null)
                .show();
    }
}