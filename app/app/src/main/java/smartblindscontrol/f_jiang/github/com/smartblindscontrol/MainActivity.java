package smartblindscontrol.f_jiang.github.com.smartblindscontrol;

import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;

import java.util.ArrayList;

import smartblindscontrol.f_jiang.github.com.smartblindscontrol.adapters.ResViewAdapter;
import smartblindscontrol.f_jiang.github.com.smartblindscontrol.data.Blind_Unit;

public class MainActivity extends AppCompatActivity implements ResViewAdapter.OnItemClicked {
    ResViewAdapter viewAdapter;
    RecyclerView my_recycler_view;

    // main activity will uses design from https://play.google.com/store/apps/details?id=com.neosmartblinds.bleapp

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });

        my_recycler_view = findViewById(R.id.my_recycler_view);
        resViewInit(null);

    }

    private void resViewInit(ArrayList<Blind_Unit> data ){
        final LinearLayoutManager viewManager = new LinearLayoutManager(this);
        viewAdapter = new ResViewAdapter(data, this);
        viewAdapter.setOnClick(this);
        my_recycler_view.setHasFixedSize(true);
        my_recycler_view.setLayoutManager(viewManager);
        my_recycler_view.setAdapter(viewAdapter);
    }

    @Override
    public void onItemClick(int position) {
        // open the activity to show the detail of the blind by position index
        // often cases start activity of that sort
    }
}
