
// Windows routine already defined, moved this implementation to
// the junk directory.

#ifdef PLATFORM_WINDOWS

// void
// mastermidibus::init()
// {

    int client;
    int num_buses = 16;
    for (int i = 0; i < num_buses; ++i)
    {
        m_buses_out[i] = new midibus(i + 1, m_queue);
        m_buses_out[i]->init_out_sub();
        m_buses_out_active[i] = true;
        m_buses_out_init[i] = true;
    }

    m_num_out_buses = num_buses;

    /* only one in */
    m_buses_in[0] = new midibus(m_num_in_buses, m_queue);
    m_buses_in[0]->init_in_sub();
    m_buses_in_active[0] = true;
    m_buses_in_init[0] = true;
    m_num_in_buses = 1;
    set_bpm(c_bpm);
    set_ppqn(c_ppqn);

    /* midi input poll descriptors */

    set_sequence_input(false, NULL);
    for (int i = 0; i < m_num_out_buses; i++)
        set_clock(i, m_init_clock[i]);

    for (int i = 0; i < m_num_in_buses; i++)
        set_input(i, m_init_input[i]);

// }

#endif // PLATFORM_WINDOWS
