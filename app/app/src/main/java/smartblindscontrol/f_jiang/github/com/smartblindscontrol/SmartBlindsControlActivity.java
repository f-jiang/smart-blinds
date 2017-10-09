package smartblindscontrol.f_jiang.github.com.smartblindscontrol;


import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;
import android.os.Bundle;
import android.preference.SwitchPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

/**
 * A {@link PreferenceActivity} that presents a set of application settings. On
 * handset devices, settings are presented as a single list. On tablets,
 * settings are split by category, with category headers shown to the left of
 * the list of settings.
 * <p>
 * See <a href="http://developer.android.com/design/patterns/settings.html">
 * Android Design: Settings</a> for design guidelines and the <a
 * href="http://developer.android.com/guide/topics/ui/settings.html">Settings
 * API Guide</a> for more information on developing a Settings UI.
 */
public class SmartBlindsControlActivity extends AppCompatPreferenceActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_smart_blinds_control);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.smart_blinds_control_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();

        if (id == R.id.action_sync) {
            // open sync activity
        } else if (id == R.id.action_calibrate) {
            startActivity(new Intent(this, CalibrateActivity.class));
        }

        return super.onOptionsItemSelected(item);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean onIsMultiPane() {
        return isXLargeTablet(this);
    }

    /**
     * This method stops fragment injection in malicious applications.
     * Make sure to deny any unknown fragments here.
     */
    protected boolean isValidFragment(String fragmentName) {
        return PreferenceFragment.class.getName().equals(fragmentName)
                || SmartBlindsControlPreferenceFragment.class.getName().equals(fragmentName);
    }

    /**
     * Helper method to determine if the device has an extra-large screen. For
     * example, 10" tablets are extra-large.
     */
    private static boolean isXLargeTablet(Context context) {
        return (context.getResources().getConfiguration().screenLayout
                & Configuration.SCREENLAYOUT_SIZE_MASK) >= Configuration.SCREENLAYOUT_SIZE_XLARGE;
    }

    /**
     * This fragment shows general preferences only. It is used when the
     * activity is showing a two-pane settings UI.
     */
    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class SmartBlindsControlPreferenceFragment
            extends PreferenceFragment
            implements SharedPreferences.OnSharedPreferenceChangeListener {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pref_smart_blinds_control);
            setHasOptionsMenu(true);
            getPreferenceScreen().getSharedPreferences()
                    .registerOnSharedPreferenceChangeListener(this);

            // here: set time dialogs' summaries, either to default value or from saved value
            SharedPreferences sharedPreferences = getPreferenceScreen().getSharedPreferences();
            PreferenceScreen preferenceScreen = getPreferenceScreen();
            Resources resources = getResources();
            int count = preferenceScreen.getPreferenceCount();

            for (int i = 0; i < count; i++) {
                Preference preference = preferenceScreen.getPreference(i);
                String key = preference.getKey();

                // TODO refactor repeated code
                if (preference != null) {
                    if (preference instanceof SwitchPreference) {
                        boolean hideTimePreference = sharedPreferences.getBoolean(key, false);

                        // TODO use String.equals()
                        if (key == resources.getString(R.string.pref_key_auto_open)) {
                            preferenceScreen.findPreference(resources.getString(R.string.pref_key_auto_open_time))
                                    .setEnabled(hideTimePreference);
                        } else if (key == resources.getString(R.string.pref_key_auto_close)) {
                            preferenceScreen.findPreference(resources.getString(R.string.pref_key_auto_close_time))
                                    .setEnabled(hideTimePreference);
                        }
                    } else if (preference instanceof TimePreference) {
                        TimePreference timePreference = (TimePreference) preference;

                        // TODO put default time in string resource xml or TimePreference class?
                        // TODO 24 time format
                        String time = sharedPreferences.getString(key, "00:00");

                        if (key == resources.getString(R.string.pref_key_auto_open_time)) {
                            timePreference.setSummary("Automatically open blinds at " + time);
                        } else if (key == resources.getString(R.string.pref_key_auto_close_time)) {
                            timePreference.setSummary("Automatically close blinds at " + time);
                        }
                    }
                }
            }
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
            getPreferenceScreen().getSharedPreferences()
                    .unregisterOnSharedPreferenceChangeListener(this);
        }

        @Override
        public boolean onOptionsItemSelected(MenuItem item) {
            int id = item.getItemId();
            if (id == android.R.id.home) {
                startActivity(new Intent(getActivity(), SmartBlindsControlActivity.class));
                return true;
            }
            return super.onOptionsItemSelected(item);
        }

        // implement onsharedpreferencechangelistener or preference.onpreferecechangelistener,
        // then update the summaries from there
        @Override
        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
            Preference preference = findPreference(key);
            Resources resources = getResources();

            // TODO refactor repeated code
            if (preference != null) {
                if (preference instanceof SwitchPreference) {
                    PreferenceScreen preferenceScreen = getPreferenceScreen();
                    boolean hideTimePreference = sharedPreferences.getBoolean(key, false);

                    if (key == resources.getString(R.string.pref_key_auto_open)) {
                        preferenceScreen.findPreference(resources.getString(R.string.pref_key_auto_open_time))
                                .setEnabled(hideTimePreference);
                    } else if (key == resources.getString(R.string.pref_key_auto_close)) {
                        preferenceScreen.findPreference(resources.getString(R.string.pref_key_auto_close_time))
                                .setEnabled(hideTimePreference);
                    }
                } else if (preference instanceof TimePreference) {
                    TimePreference timePreference = (TimePreference) preference;

                    // TODO put default time in string resource xml or TimePreference class?
                    // TODO 24 time format
                    String time = sharedPreferences.getString(key, "00:00");

                    if (key == resources.getString(R.string.pref_key_auto_open_time)) {
                        timePreference.setSummary("Automatically open blinds at " + time);
                    } else if (key == resources.getString(R.string.pref_key_auto_close_time)) {
                        timePreference.setSummary("Automatically close blinds at " + time);
                    }
                }
            }
        }

    }

}
