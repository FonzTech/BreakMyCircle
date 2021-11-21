package it.fonztech.breakmycircle;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.Nullable;

public final class NotificationActivity extends Activity {
    protected static final String TAG = NotificationActivity.class.getName();
    protected static final String INTENT_EXTRA_PREFIX = "game_";

    @Override
    protected final void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final EngineActivity ea = EngineActivity.CURRENT_INSTANCE.get();
        if (ea != null) {
            Log.d(TAG, "Engine is already running.");
            copyGameIntentData(getIntent(), ea.getIntent());
        }
        else {
            Log.d(TAG, "Engine needs to be started up.");
            final Intent intent = new Intent(this, MainActivity.class);
            copyGameIntentData(getIntent(), intent);
            startActivity(intent);
        }
        finish();
        overridePendingTransition(0, 0);
    }

    /**
     * Copy game notification data from source intent to a destination one.
     *
     * @param src the source intent to copy the data from.
     * @param dst the destination intent to copy the data to.
     */
    protected final void copyGameIntentData(final Intent src, final Intent dst) {
        final Bundle extras = src.getExtras();
        if (extras != null && !extras.isEmpty()) {
            for (final String key : extras.keySet()) {
                if (key.startsWith(INTENT_EXTRA_PREFIX)) {
                    dst.putExtra(key, src.getStringExtra(key));
                }
            }
        }
    }
}