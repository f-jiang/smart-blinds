package smartblindscontrol.f_jiang.github.com.smartblindscontrol;

import android.support.v4.app.NavUtils;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.ImageButton;

public class CalibrateActivity extends AppCompatActivity {

    ImageButton mUpButton;
    ImageButton mDownButton;
    Button mSetMinButton;
    Button mSetMaxButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_calibrate);
        ActionBar actionBar = this.getSupportActionBar();

        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

        mUpButton = findViewById(R.id.up_button);
        mDownButton = findViewById(R.id.down_button);
        mSetMinButton = findViewById(R.id.set_min_button);
        mSetMaxButton = findViewById(R.id.set_max_button);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            NavUtils.navigateUpFromSameTask(this);
        }
        return super.onOptionsItemSelected(item);
    }

}
