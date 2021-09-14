package it.fonztech.breakmycircle;

import android.app.NativeActivity;
import android.content.Intent;
import android.net.Uri;

public class GameActivity extends NativeActivity {
    protected static final String URL_VOTE_ME = "https://play.google.com/store/apps/details?id=it.fonztech.breakmycircle";
    protected static final String URL_OTHER_APPS = "https://play.google.com/store/apps/developer?id=FonzTech";

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
}