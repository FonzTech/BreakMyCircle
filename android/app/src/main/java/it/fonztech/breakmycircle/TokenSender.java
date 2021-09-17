package it.fonztech.breakmycircle;

import android.util.Log;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;

import javax.net.ssl.HttpsURLConnection;

public final class TokenSender extends Thread {
    protected static final String TAG = TokenSender.class.getSimpleName();

    protected final String mToken;

    public TokenSender(final String token) {
        super(TAG);
        mToken = token;
    }

    @Override
    public final void run() {
        HttpsURLConnection c = null;
        try {
            final URL url = new URL(Utility.BACKEND_URL + "firebase.php");
            c = (HttpsURLConnection) url.openConnection();
            c.setReadTimeout(15000);
            c.setConnectTimeout(30000);
            c.setDoInput(true);
            c.setDoOutput(true);

            {
                final JSONObject json = new JSONObject();
                json.put("token", mToken);

                final OutputStream os = c.getOutputStream();
                os.write(json.toString().getBytes());
                os.close();
            }

            final ByteArrayOutputStream baos = new ByteArrayOutputStream();
            final InputStream is = c.getInputStream();
            final byte[] buf = new byte[1024];
            int len;
            while ((len = is.read(buf)) > 0) {
                baos.write(buf, 0, len);
            }
            is.close();
            baos.close();

            final JSONObject json = new JSONObject(new String(baos.toByteArray()));
            Log.d(TAG, json.toString());
        }
        catch (final Exception e) {
            e.printStackTrace();
        }
        finally {
            if (c != null) {
                c.disconnect();
            }
        }
    }
}