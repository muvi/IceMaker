package muviz.icecontroller;

import android.bluetooth.BluetoothAdapter;
import android.util.Log;
import android.os.Handler;
import android.os.Message;
import android.os.Bundle;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

//import android

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.util.UUID;

/**
 * Created by christian on 12.02.18.
 */

public class IceBluetooth {
    // Debugging
    private static final String TAG = "BluetoothService";

    // Name for the SDP record when creating server socket
    private static final String NAME = "BluetoothChatSecure";

    // Unique UUID for this application
    private static final UUID SPP_UUID =
            UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    // Member fields
    private final BluetoothAdapter mAdapter;
    private BluetoothSocket mSocket = null;
    private BluetoothDevice mDevice = null;
    private final Handler mHandler;
    private int mState;

    /**
     * Constructor. Prepares a new BluetoothChat session.
     *
     * @param handler A Handler to send messages back to the UI Activity
     */
    public IceBluetooth(Handler handler) {
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mHandler = handler;
    }

    public boolean connect(BluetoothDevice device){
        mAdapter.cancelDiscovery();
        try {
            mSocket = device.createRfcommSocketToServiceRecord(SPP_UUID);
            mSocket.connect();

        } catch (IOException e) {
            mSocket = null;
            return false;
        }
        mDevice = device;
        return true;
    }

    public void disconnect(){
        if (mSocket != null) {
            try {
                mSocket.close();
                mSocket = null;
                mDevice = null;
            } catch (IOException e) {

            }
        }
    }

    public boolean isConnected(){
        return (mSocket != null) && mSocket.isConnected();
    }

    public boolean sendCheck() throws IOException {
        mSocket.getOutputStream().write(Const.BT_MSGM_CHECK);
        byte[] bytes = new byte[1];
        mSocket.getInputStream().read(bytes, 0, 1);
        return bytes[0] == Const.BT_MSGS_CHECKRESPONSE;
    }

    /*private class ReceiveThread extends Thread {
        public ReceiveThread() {

        }

    }*/
}
