package smartblindscontrol.f_jiang.github.com.smartblindscontrol.adapters;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;

import java.util.ArrayList;

import smartblindscontrol.f_jiang.github.com.smartblindscontrol.data.Blind_Unit;

/**
 * Created by eugen on 8/24/2018.
 */

public class ResViewAdapter extends RecyclerView.Adapter<ResViewAdapter.ViewHolder>{
    ArrayList<Blind_Unit> myDataset;
    Context context;

    public ResViewAdapter(ArrayList<Blind_Unit> myDataset, Context context){
        this.myDataset = myDataset;
        this.context = context;
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
    public int getItemCount(){
        return myDataset.size;
    }

    private OnItemClicked onClick;
    public void setOnClick(OnItemClicked onClick) {
        this.onClick = onClick;
    }


    interface OnItemClicked {
        void onItemClick(int position);
    }

}
