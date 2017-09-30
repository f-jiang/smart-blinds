package smartblindscontrol.f_jiang.github.com.smartblindscontrol;

import android.content.Context;
import android.content.res.TypedArray;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.TimePicker;

/**
 * Created by feilan on 29/09/17.
 */

public class TimePreference extends DialogPreference {

    private int lastHour = 0;
    private int lastMinute = 0;

    private TimePicker mTimePicker = null;

    public static int getHour(String time) {
        String[] pieces = time.split(":");
        return Integer.parseInt(pieces[0]);
    }

    public static int getMinute(String time) {
        String[] pieces = time.split(":");
        return Integer.parseInt(pieces[1]);
    }

    public TimePreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setPositiveButtonText("Set");
        setNegativeButtonText("Cancel");
    }

    @Override
    protected View onCreateDialogView() {
        mTimePicker = new TimePicker(getContext());
        return mTimePicker;
    }

    @Override
    protected void onBindDialogView(View v) {
        super.onBindDialogView(v);
        mTimePicker.setHour(lastHour);
        mTimePicker.setMinute(lastMinute);
    }

    @Override
    protected void onDialogClosed(boolean positiveResult) {
        super.onDialogClosed(positiveResult);

        if (positiveResult) {
            lastHour = mTimePicker.getHour();
            lastMinute = mTimePicker.getMinute();

            setSummary(getSummary());

            String lastMinuteString = String.valueOf(lastMinute);
            String time = String.valueOf(lastHour) + ":"
                    + (lastMinuteString.length() == 1 ? "0" + lastMinuteString : lastMinuteString);

            if (callChangeListener(time)) {
                persistString(time);
            }
        }
    }

    @Override
    protected Object onGetDefaultValue(TypedArray a, int index) {
        return a.getString(index);
    }

    @Override
    protected void onSetInitialValue(boolean restoreValue, Object defaultValue) {
        String time = null;

        if (restoreValue) {
            if (defaultValue == null) {
                time = getPersistedString("00:00");
            } else {
                time = getPersistedString(defaultValue.toString());
            }
        } else {
            time = defaultValue.toString();
        }

        lastHour = getHour(time);
        lastMinute = getMinute(time);
    }

}
