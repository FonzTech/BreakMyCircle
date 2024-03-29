package it.fonztech.breakmycircle;

import android.content.Context;
import android.util.Log;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.net.URL;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;

import javax.net.ssl.HttpsURLConnection;

public final class TokenSender extends Thread {
    protected static final String TAG = TokenSender.class.getSimpleName();

    protected final String mToken;
    protected final WeakReference<Context> contextRef;

    public TokenSender(final Context context, final String token) {
        super(TAG);
        contextRef = new WeakReference<>(context);
        mToken = token;
    }

    @Override
    public final void run() {
        HttpsURLConnection c = null;
        try {
            final URL url = new URL(Utility.BACKEND_URL + Utility.BACKEND_FIREBASE);
            c = (HttpsURLConnection) url.openConnection();
            c.setReadTimeout(15000);
            c.setConnectTimeout(30000);
            c.setRequestMethod("POST");
            c.setDoInput(true);
            c.setDoOutput(true);

            {
                final StringBuilder sb = Utility.getBasicApiPayload(contextRef != null ? contextRef.get() : null);
                sb.append("&token=").append(URLEncoder.encode(mToken, StandardCharsets.UTF_8.name()));

                final OutputStream os = c.getOutputStream();
                os.write(sb.toString().getBytes());
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
            Log.d(TAG, "FCM Register Token - " + json.toString());
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