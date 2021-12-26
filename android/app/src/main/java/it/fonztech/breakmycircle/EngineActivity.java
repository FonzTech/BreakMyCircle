package it.fonztech.breakmycircle;

import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

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

import java.io.File;
import java.io.IOException;
import java.lang.ref.WeakReference;

public abstract class EngineActivity extends NativeActivity implements OnInitializationCompleteListener, OnUserEarnedRewardListener, OnCompleteListener<String> {
    private static final String TAG = EngineActivity.class.getSimpleName();

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
            setRewardedInfo(null, 0);
            showError(loadAdError.getMessage());
        }
    };

    protected final RewardedAdLoadCallback rewardedAdLoadCallback = new RewardedAdLoadCallback() {
        @Override
        public void onAdFailedToLoad(@NonNull final LoadAdError loadAdError) {
            setRewardedInfo(null, 0);
            showError(Utility.DEBUG ? loadAdError.getMessage() : null);
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
            showError(Utility.DEBUG ? adError.getMessage() : null);
        }

        @Override
        public final void onAdShowedFullScreenContent() {
        }
    };

    public static WeakReference<EngineActivity> CURRENT_INSTANCE = new WeakReference<>(null);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        CURRENT_INSTANCE = new WeakReference<>(this);

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
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        CURRENT_INSTANCE = new WeakReference<>(null);
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
        new TokenSender(this, token).start();
    }

    /**
     * Abstract method to implement in the child activity, who runs the game library.
     * This method is used to pass rewards to your game.
     *
     * @param type string representing the type, passed from Ads settings (or a mock).
     * @param amount how much rewards to give to the player, passed from Ads settings (or a mock).
     */
    protected abstract void setRewardedInfo(final String type, final int amount);

    /**
     * Abstract method to implement in the child activity.
     *
     * @return backend API URL for the game (to sync settings, stats, etc...).
     */
    protected abstract String getBackendUrl();

    /**
     * Open any supported URI in Android, using the default user/system intent catcher.
     *
     * @param url string representing the URI/URL to open. Intent catcher is chosen by
     *            the provided URI/URL scheme.
     */
    protected final void openUrl(final String url) {
        final Intent i = new Intent(Intent.ACTION_VIEW);
        i.setData(Uri.parse(url));
        startActivity(i);
    }

    /**
     * This method is used to show an Interstitial Ad. It can be called from the Java
     * part or even directly from the C++ part (hence the "unused" warning suppression).
     * If it was NOT possible to show the ad (due to mis-configuration, no network, etc...)
     * no dialog is shown. Instead, all the rewards to the user is cleared.
     */
    @SuppressWarnings("unused")
    protected final void showInterstitial() {
        if (mCanShowAds) {
            try {
                if (Utility.isNetworkAvailable(this)) {
                    runOnUiThread(() -> {
                        final AdRequest adRequest = new AdRequest.Builder().build();
                        InterstitialAd.load(EngineActivity.this, Utility.DEBUG ? INTERSTITIAL_AD_DEV : INTERSTITIAL_AD_PROD, adRequest, interstitialAdLoadCallback);
                    });
                }
                else {
                    throw new AdShowError();
                }
            }
            catch (final AdShowError e) {
                setRewardedInfo(ADS_NOT_AVAILABLE_TYPE, 0);
            }
        }
    }

    /**
     * This method is used to show a Rewarded Interstitial Ad. It can be called from the
     * Java part or even directly from the C++ part (hence the "unused" warning suppression).
     * If it was NOT possible to show the ad (due to mis-configuration, no network, etc...)
     * then a dialog with an error is shown.
     */
    @SuppressWarnings("unused")
    protected final void watchAdForPowerup() {
        try {
            if (mCanShowAds && Utility.isNetworkAvailable(this)) {
                runOnUiThread(() -> {
                    final AdRequest adRequest = new AdRequest.Builder().build();
                    RewardedAd.load(EngineActivity.this, Utility.DEBUG ? REWARDED_AD_DEV : REWARDED_AD_PROD, adRequest, rewardedAdLoadCallback);
                });
            }
            else {
                throw new AdShowError();
            }
        }
        catch (final AdShowError e) {
            setRewardedInfo(ADS_NOT_AVAILABLE_TYPE, 0);
            new AlertDialog.Builder(EngineActivity.this)
                    .setTitle(R.string.app_name)
                    .setMessage(R.string.ad_not_available)
                    .setPositiveButton(android.R.string.ok, null)
                    .show();
        }
    }

    /**
     * Get status bar height. Useful to be used in the engine to avoid putting GUI under the
     * status/notification bar in Android.
     *
     * @return number of pixels occupied by the system status/notification bar.
     */
    protected final int statusBarHeight() {
        int result = 0;
        int resourceId = getResources().getIdentifier("status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            result = getResources().getDimensionPixelSize(resourceId);
        }
        return result;
    }

    /**
     * Show error dialog, with a given message or a default one (if null), with a single "OK"
     * button.
     *
     * @param message nullable message. If null, then a default message is shown.
     */
    protected final void showError(@Nullable final String message) {
        new AlertDialog.Builder(EngineActivity.this)
                .setTitle(R.string.app_name)
                .setMessage(message != null ? message : getString(R.string.ad_error))
                .setPositiveButton(android.R.string.ok, null)
                .show();
    }

    /**
     * Custom class for ad show exception.
     */
    protected static class AdShowError extends RuntimeException {
        protected AdShowError() {
            super();
        }
    }
}