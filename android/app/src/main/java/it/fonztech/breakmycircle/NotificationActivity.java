package it.fonztech.breakmycircle;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import androidx.annotation.Nullable;

public final class NotificationActivity extends Activity {
    @Override
    protected final void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (!EngineActivity.IS_ACTIVE) {
            startActivity(new Intent(this, MainActivity.class));
        }
        finish();
        overridePendingTransition(0, 0);
    }
}