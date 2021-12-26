package it.fonztech.breakmycircle;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.Nullable;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;

public final class AssetsUnpacker extends Thread {
    private static final String TAG = AssetsUnpacker.class.getSimpleName();
    private static final String ASSET_PREFERENCES = "ASSET_PREFERENCES";

    private final Context context;
    private final Runnable callback;
    private Exception exception;

    public AssetsUnpacker(final Context context, final Runnable callback) {
        super(AssetsUnpacker.class.getName());
        this.context = context;
        this.callback = callback;
    }

    @Override
    public final void run() {
        // Copy all packed assets into internal app storage
        try {
            final SharedPreferences prefs = context.getSharedPreferences(ASSET_PREFERENCES, Context.MODE_PRIVATE);
            final SharedPreferences.Editor editor = prefs.edit();

            final JSONObject json = new JSONObject(getAssetsJson());

            final Iterator<String> iterator = json.keys();
            while (iterator.hasNext()) {
                // Check if processing was interrupted
                if (isInterrupted()) {
                    break;
                }

                // Get next filename
                final String fname = iterator.next();

                // Get info for this asset
                final JSONObject assetJson = json.getJSONObject(fname);

                // Prepare for unpack
                final File dest = new File(context.getFilesDir(), fname);
                final long assetVersion = assetJson.getLong("version");
                final long assetSize = assetJson.getLong("size");

                // Check if asset file is up to date
                if (prefs.getLong(fname, 0L) >= assetVersion) {
                    // Check for size
                    if (dest.length() == assetSize) {
                        // This file is correct
                        Log.d(TAG, "Asset " + fname + " (" + assetVersion + ") already exists in " + dest.getAbsolutePath());
                        continue;
                    }

                    Log.d(TAG, "Asset " + fname + " (" + assetVersion + ") appear to be inconsistent in size.");
                }

                // Unpack asset procedure
                {
                    Log.d(TAG, "Copying asset " + fname + " (" + assetVersion + ") into " + dest.getAbsolutePath());

                    // Create parent directories for this file
                    {
                        final File parentDir = dest.getParentFile();
                        //noinspection ConstantConditions
                        if (!parentDir.exists() && !parentDir.mkdirs()){
                            throw new RuntimeException("Could not make parent directories for " + dest.getAbsolutePath());
                        }
                    }

                    // Copy to destination file
                    final InputStream is = context.getAssets().open(fname);
                    final FileOutputStream fos = new FileOutputStream(dest);

                    final byte[] buffer = new byte[1024 * 8];
                    for (int length; (length = is.read(buffer)) != -1; ) {
                        fos.write(buffer, 0, length);
                    }

                    fos.close();
                    is.close();

                    // Save asset info
                    editor.putLong(fname, assetVersion);

                    Thread.sleep(50L);
                }
            }

            editor.apply();
        }
        catch (Exception e) {
            throw new RuntimeException(e.getMessage() != null ? e.getMessage() : "Unknown exception error");
        }

        // Run callback
        if (!isInterrupted()) {
            callback.run();
        }
    }

    /**
     * Get exception occurred during assets unpacking.
     *
     * @return nullable exception.
     */
    @Nullable
    public final Exception getException() {
        return exception;
    }

    /**
     * Get entire JSON string, from the "assets" file containing all the assets
     * and their version number, directly from the app bundle.
     *
     * @return entire JSON string representing the "assets" file content.
     *
     * @throws IOException any IO exception thrown during file read.
     */
    protected final String getAssetsJson() throws IOException {
        final InputStream is = context.getResources().openRawResource(R.raw.assets);
        final ByteArrayOutputStream baos = new ByteArrayOutputStream();

        final byte[] buffer = new byte[1024 * 10];
        for (int length; (length = is.read(buffer)) != -1; ) {
            baos.write(buffer, 0, length);
        }

        baos.close();
        is.close();

        return new String(baos.toByteArray());
    }
}