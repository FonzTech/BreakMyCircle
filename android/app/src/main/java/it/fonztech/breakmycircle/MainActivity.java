package it.fonztech.breakmycircle;

import android.app.NativeActivity;
import android.os.Bundle;

import java.io.File;

public class MainActivity extends NativeActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        {
            String dir = getFilesDir().getAbsolutePath();
            if (!dir.endsWith(File.separator))
            {
                dir += File.separator;
            }
            getIntent().putExtra("asset_dir", dir);
        }
    }
}