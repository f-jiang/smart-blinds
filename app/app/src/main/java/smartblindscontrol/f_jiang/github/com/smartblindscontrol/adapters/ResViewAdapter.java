package smartblindscontrol.f_jiang.github.com.smartblindscontrol.adapters;

import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;

/**
 * Created by eugen on 8/24/2018.
 */

public class ResViewAdapter extends RecyclerView.Adapter<ResViewAdapter.ViewHolder>{

    public ResViewAdapter(){

    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        public ViewHolder(View itemView) {
            super(itemView);
        }
    }
    @Override
    public ResViewAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        return null;
    }

    @Override
    public void onBindViewHolder(ResViewAdapter.ViewHolder holder, int position) {

    }

    @Override
    public int getItemCount() {
        return 0;
    }
}
