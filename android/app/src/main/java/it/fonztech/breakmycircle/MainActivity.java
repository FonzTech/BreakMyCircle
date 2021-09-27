package it.fonztech.breakmycircle;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;

import javax.net.ssl.HttpsURLConnection;

public class MainActivity extends EngineActivity implements Runnable, DialogInterface.OnClickListener {
    protected AlertDialog mDialog;

    @Override
    protected final void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        new Thread(this).start();
    }

    @Override
    public void onBackPressed() {
        if (mDialog != null) {
            mDialog.cancel();
            mDialog = null;
        }
        else {
            mDialog = new AlertDialog.Builder(this)
                    .setTitle(R.string.app_name)
                    .setMessage(R.string.exit_alert)
                    .setPositiveButton(R.string.yes, this)
                    .setNegativeButton(R.string.no, this)
                    .create();
            mDialog.show();
        }
    }

    @Override
    protected final void setRewardedInfo(final String type, final int amount) {
        if ("powerup".equalsIgnoreCase(type))
        {
            setPowerupInfo(amount);
        }
        else if (ADS_NOT_AVAILABLE_TYPE.equalsIgnoreCase(type))
        {
            setPowerupInfo(0);
        }
        else
        {
            setPowerupInfo(Utility.DEBUG ? 1 : amount);
        }
    }

    @Override
    protected final String getBackendUrl() {
        return Utility.BACKEND_URL + "api.php";
    }

    @Override
    public void run() {
        HttpsURLConnection c = null;
        try {
            final URL url = new URL(getBackendUrl());
            c = (HttpsURLConnection) url.openConnection();
            c.setReadTimeout(15000);
            c.setConnectTimeout(30000);
            c.setDoInput(true);
            c.setDoOutput(true);

            {
                final JSONObject json = new JSONObject();
                json.put("version", getPackageManager().getPackageInfo(getPackageName(), 0).versionCode);

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
            mCanShowAds = json.getBoolean("canShowAds");
            mPlayAdThreshold = json.getInt("playAdThreshold");

            getIntent().putExtra("play_ad_threshold", Integer.toString(mPlayAdThreshold));
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

    @Override
    public void onClick(DialogInterface dialogInterface, int i) {
        switch (i) {
            case AlertDialog.BUTTON_POSITIVE:
                super.onBackPressed();
                break;

            case AlertDialog.BUTTON_NEGATIVE:
                onBackPressed();
                break;
        }
    }

    protected final void setPowerupInfo(final int rewardAmount) {
        getIntent().putExtra("game_powerup_amount", Integer.toString(rewardAmount));
        getIntent().putExtra("game_powerup_expire", Long.toString(System.currentTimeMillis() + 1000L));
    }

    @SuppressWarnings("unused")
    protected final void clearPowerupData() {
        getIntent().removeExtra("game_powerup_amount");
        getIntent().removeExtra("game_powerup_expire");
    }
}