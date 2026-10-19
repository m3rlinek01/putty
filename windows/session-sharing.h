/* Author: m3rlinek. */
/* GUI policy for duplicating authenticated transports, independent of WinAPI. */
#ifndef PUTTY_GUI_SESSION_SHARING_H
#define PUTTY_GUI_SESSION_SHARING_H

inline void gui_session_downstream(Conf *conf)
{
    conf_set_bool(conf, CONF_ssh_connection_sharing, true);
    conf_set_bool(conf, CONF_ssh_connection_sharing_upstream, false);
    conf_set_bool(conf, CONF_ssh_connection_sharing_downstream, true);
    conf_set_bool(conf, CONF_ssh_share_required, true);
}

inline Conf *gui_duplicate_session_conf(Conf *source, bool authenticated)
{
    Conf *copy = conf_copy(source);
    if (authenticated && conf_get_int(source, CONF_protocol) == PROT_SSH &&
        conf_get_bool(source, CONF_ssh_duplicate_session) &&
        *conf_get_str(source, CONF_ssh_share_id)) {
        gui_session_downstream(copy);
    } else {
        /* A duplicate of a closed/unconnected window starts normally. Never
         * accidentally retain the old authenticated endpoint in that case. */
        conf_set_str(copy, CONF_ssh_share_id, "");
        conf_set_bool(copy, CONF_ssh_share_required, false);
    }
    return copy;
}

inline void gui_prepare_session_sharing(Conf *conf, const char *fresh_id)
{
    if (conf_get_int(conf, CONF_protocol) == PROT_SSH &&
        conf_get_bool(conf, CONF_ssh_share_required)) {
        gui_session_downstream(conf);
        if (!*conf_get_str(conf, CONF_ssh_share_id))
            conf_set_bool(conf, CONF_ssh_connection_sharing, false);
        return;
    }
    conf_set_str(conf, CONF_ssh_share_id, "");
    if (conf_get_int(conf, CONF_protocol) == PROT_SSH && *fresh_id &&
        (conf_get_bool(conf, CONF_ssh_file_transfer) ||
         conf_get_bool(conf, CONF_ssh_duplicate_session))) {
        conf_set_str(conf, CONF_ssh_share_id, fresh_id);
        conf_set_bool(conf, CONF_ssh_connection_sharing, true);
        conf_set_bool(conf, CONF_ssh_connection_sharing_upstream, true);
        conf_set_bool(conf, CONF_ssh_connection_sharing_downstream, false);
    }
}
#endif
