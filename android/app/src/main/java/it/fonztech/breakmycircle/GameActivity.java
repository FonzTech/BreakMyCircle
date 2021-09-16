package it.fonztech.breakmycircle;

import android.app.NativeActivity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;

import androidx.annotation.NonNull;

import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.FullScreenContentCallback;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;
import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;

public class GameActivity extends NativeActivity implements OnInitializationCompleteListener {
    protected static final String URL_VOTE_ME = "https://play.google.com/store/apps/details?id=it.fonztech.breakmycircle";
    protected static final String URL_OTHER_APPS = "https://play.google.com/store/apps/developer?id=FonzTech";

    protected static final String INTERSTITIAL_AD_DEV = "ca-app-pub-3940256099942544/1033173712";
    protected static final String REWARDED_AD_DEV = "ca-app-pub-3940256099942544/5354046379";

    protected static final String INTERSTITIAL_AD_PROD = "ca-app-pub-3837498848455030/3033316978";
    protected static final String REWARDED_INT_AD_PROD = "ca-app-pub-3940256099942544/5354046379";

    protected final InterstitialAdLoadCallback interstitialAdLoadCallback = new InterstitialAdLoadCallback() {
        @Override
        public final void onAdLoaded(@NonNull final InterstitialAd interstitialAd) {
            interstitialAd.setFullScreenContentCallback(fullScreenContentCallback);
            interstitialAd.show(GameActivity.this);
        }

        @Override
        public final void onAdFailedToLoad(@NonNull final LoadAdError loadAdError) {
        }
    };

    protected final FullScreenContentCallback fullScreenContentCallback = new FullScreenContentCallback() {
        @Override
        public final void onAdDismissedFullScreenContent() {
        }

        @Override
        public final void onAdFailedToShowFullScreenContent(@SuppressWarnings("NullableProblems") final AdError adError) {
        }

        @Override
        public final void onAdShowedFullScreenContent() {
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MobileAds.initialize(this, this);
    }

    @Override
    public final void onInitializationComplete(@SuppressWarnings("NullableProblems") final InitializationStatus initializationStatus) {
    }

    @SuppressWarnings("unused")
    protected final void gameVoteMe() {
        openUrl(URL_VOTE_ME);
    }

    @SuppressWarnings("unused")
    protected final void gameOtherApps() {
        openUrl(URL_OTHER_APPS);
    }

    protected final void openUrl(final String url) {
        final Intent i = new Intent(Intent.ACTION_VIEW);
        i.setData(Uri.parse(url));
        startActivity(i);
    }

    protected final void showInterstitial() {
        final AdRequest adRequest = new AdRequest.Builder().build();
        InterstitialAd.load(this, BuildConfig.DEBUG ?INTERSTITIAL_AD_DEV : INTERSTITIAL_AD_PROD, adRequest, interstitialAdLoadCallback);
    }
}