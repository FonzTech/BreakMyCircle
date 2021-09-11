package it.fonztech.breakmycircle;

import android.app.NativeActivity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;

public class MainActivity extends NativeActivity {
    private static final String TAG = MainActivity.class.getSimpleName();
    private static final String ASSET_PREFERENCES = "ASSET_PREFERENCES";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Set base directory for assets
        {
            // final float density = Math.max(1.0f, getResources().getDisplayMetrics().density);
            final float density = 1.0f;
            getIntent().putExtra("density", Float.toString(density));
        }

        {
            final int height = statusBarHeight();
            getIntent().putExtra("canvas_vertical_height", Integer.toString(height));
        }

        {
            String assetDir = getFilesDir().getAbsolutePath();
            if (!assetDir.endsWith(File.separator))
            {
                assetDir += File.separator;
            }
            getIntent().putExtra("asset_dir", assetDir);
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

    protected final int statusBarHeight() {
        return (int) (24.0f * getResources().getDisplayMetrics().density);
    }
}