

#if 0
    Commented code from:

    void sequence::stretch_selected (long a_delta_tick)

    event *on, *off, new_on, new_off;

    lock();

    std::list<event>::iterator i;

    int old_len = 0, new_len = 0;
    int first_ev = 0x7fffffff;
    int last_ev = 0x00000000;

    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if ((*i).is_selected() &&
                (*i).is_note_on() &&
                (*i).is_linked())
        {

            on = &(*i);
            off = (*i).get_linked();

            if (on->get_timestamp() < first_ev)
            {
                first_ev = on->get_timestamp();
            }
            if (off->get_timestamp() > last_ev)
            {
                last_ev = off->get_timestamp();
            }
        }
    }

    old_len = last_ev - first_ev;
    new_len = old_len + a_delta_tick;
    float ratio = float(new_len) / float(old_len);

    if (new_len > 1)
    {

        mark_selected();

        for (i = m_list_event.begin(); i != m_list_event.end(); i++)
        {
            if ((*i).is_marked() &&
                    (*i).is_note_on() &&
                    (*i).is_linked())
            {

                on = &(*i);
                off = (*i).get_linked();

                /* copy & scale event */
                new_on = *on;
                new_on.set_timestamp(long((on->get_timestamp() - first_ev) * ratio) + first_ev);
                new_off = *off;
                new_off.set_timestamp(long((off->get_timestamp() - first_ev) * ratio) + first_ev);

                new_on.unmark();
                new_off.unmark();

                add_event(&new_on);
                add_event(&new_off);
            }
        }

        remove_marked();
        verify_and_link();
    }

    unlock();

#endif
