package muviz.icecontroller;

/**
 * Created by christian on 13.02.18.
 */

public interface Const {
    public final byte BT_MSGM_CHECK         = 'C';
    public final byte BT_MSGM_QUERYSTATE    = 'Q';
    public final byte BT_MSGM_QUERYSTATES   = 'Y';
    public final byte BT_MSGM_SETSTATES     = 'S';

    public final byte BT_MSGS_CHECKRESPONSE = 'K';
    public final byte BT_MSGS_STATEINFO     = 'I';
    public final byte BT_MSGS_STATES        = 'S';
    public final byte BT_MSGS_SETSTATESOK   = 'O';
    public final byte BT_MSGS_SETSTATESERROR= 'E';
}
