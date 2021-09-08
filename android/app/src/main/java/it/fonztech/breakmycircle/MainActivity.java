package it.fonztech.breakmycircle;

import android.app.NativeActivity;
import android.os.Bundle;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class MainActivity extends NativeActivity {
    private static final String TAG = MainActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Set base directory for assets
        {
            String assetDir = getFilesDir().getAbsolutePath();
            if (!assetDir.endsWith(File.separator))
            {
                assetDir += File.separator;
            }
            getIntent().putExtra("asset_dir", assetDir);
        }

        // Copy all packed assets into internal app storage
        final String[] dirs = { "audios", "fonts", "paths", "rooms", "scenes", "shaders", "textures" };

        try {
            for (final String dir : dirs) {
                for (final String file : getAssets().list(dir)) {
                    final String fname = dir + File.separator + file;
                    final File dest = new File(getFilesDir(), fname);
                    if (dest.exists()) {
                        Log.d(TAG, "Asset " + fname + " already exists in " + dest.getAbsolutePath());
                    }
                    else {
                        Log.d(TAG, "Copying asset " + fname + " into " + dest.getAbsolutePath());

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
                    }
                }
            }
        }
        catch (Exception e) {
            throw new RuntimeException(e.getMessage() != null ? e.getMessage() : "Unknown exception error");
        }
    }
}