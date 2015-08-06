/*
 *  This file is part of seq24/sequencer24.
 *
 *  seq24 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  seq24 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with seq24; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * \file          midifile.cpp
 *
 *  This module declares/defines the base class for MIDI files.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-05
 * \license       GNU GPLv2 or above
 *
 */

#include <fstream>

#include "easy_macros.h"                // errprint() macro
#include "perform.h"                    // must precede midifile.h !
#include "midifile.h"
#include "sequence.h"

/**
 *  Principal constructor.
 */

midifile::midifile (const std::string & a_name)
 :
    m_pos           (0),
    m_name          (a_name),
    m_data          (),
    m_char_list     ()
{
    // empty body
}

/**
 *  A rote destructor.
 */

midifile::~midifile ()
{
    // empty body
}

/**
 *  Reads 4 bytes of data using read_byte().
 *
 * \warning
 *      This code looks endian-dependent.
 */

unsigned long
midifile::read_long ()
{
    unsigned long result = 0;
    result += (read_byte() << 24);
    result += (read_byte() << 16);
    result += (read_byte() << 8);
    result += (read_byte());
    return result;
}

/**
 *  Reads 2 bytes of data using read_byte().
 *
 * \warning
 *      This code looks endian-dependent.
 */

unsigned short
midifile::read_short ()
{
    unsigned short result = 0;
    result += (read_byte() << 8);
    result += (read_byte());
    return result;
}

/**
 *  Reads 1 byte of data directly into the m_data vector, incrementing
 *  m_pos after doing so.
 */

unsigned char
midifile::read_byte ()
{
    return m_data[m_pos++];
}

/**
 *  Reads a variable number of bytes, while bit 7 is set.
 */

unsigned long
midifile::read_var ()
{
    unsigned long result = 0;
    unsigned char c;
    while (((c = read_byte()) & 0x80) != 0x00)      /* while bit 7 is set  */
    {
        result <<= 7;                               /* shift result 7 bits */
        result += (c & 0x7F);                       /* add bits 0-6        */
    }

    /* bit was clear */

    result <<= 7;
    result += (c & 0x7F);
    return result;
}

/**
 *  This function opens a binary file.
 */

bool
midifile::parse (perform * a_perf, int a_screen_set)
{
    std::ifstream file(m_name.c_str(), ios::in | ios::binary | ios::ate);
    if (! file.is_open())
    {
        errprint("Error opening MIDI file");
        return false;
    }

    int file_size = file.tellg();                   /* get end offset ??  */
    file.seekg(0, ios::beg);                        /* seek to start      */
    try
    {
        m_data.resize(file_size);                   /* allocate more data */
    }
    catch (std::bad_alloc & ex)
    {
        errprint("Memory allocation failed");
        return false;
    }
    file.read((char *) &m_data[0], file_size);
    file.close();
    m_pos = 0;

    unsigned long ID;                               /* chunk info         */
    unsigned long TrackLength;
    unsigned long Delta;                            /* time               */
    unsigned long RunningTime;
    unsigned long CurrentTime;
    unsigned short Format;                          /* 0,1,2              */
    unsigned short NumTracks;
    unsigned short ppqn;
    unsigned short perf;
    char TrackName[256];                            /* track name from file */
    int i;                                          /* used in small loops  */
    sequence * seq;                                 /* sequence pointer     */
    event e;

    ID = read_long();                               /* read in header */
    TrackLength = read_long();
    Format = read_short();
    NumTracks = read_short();
    ppqn = read_short();
    if (ID != 0x4D546864)                           /* magic number 'MThd' */
    {
        fprintf(stderr, "Invalid MIDI header detected: %8lX\n", ID);
        return false;
    }
    if (Format != 1)                                /* only support format 1 */
    {
        fprintf(stderr, "Unsupported MIDI format detected: %d\n", Format);
        return false;
    }

    /*
     * We should be good to load now, for each Track in the MIDI file.
     */

    for (int curTrack = 0; curTrack < NumTracks; curTrack++)
    {
        bool done = false;                          /* done for each track */
        perf = 0;
        unsigned char status = 0, type, data[2], laststatus; /* events */
        long len;
        unsigned long proprietary = 0;

        ID = read_long();                           /* Get ID + Length */
        TrackLength = read_long();
        if (ID == 0x4D54726B)                       /* magic number 'MTrk' */
        {
            /*
             * We know we have a good track, so we can create a new
             * sequence to dump it to.
             */

            seq = new sequence();
            if (seq == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                return false;
            }
            seq->set_master_midi_bus(&a_perf->m_master_bus);    /* set buss  */
            RunningTime = 0;                    /* reset time                */
            while (! done)                      /* get each event in the Trk */
            {
                Delta = read_var();             /* get time delta            */
                laststatus = status;
                status = m_data[m_pos];         /* get status                */
                if ((status & 0x80) == 0x00)    /* is it a status bit ?      */
                    status = laststatus;        /* no, it's a running status */
                else
                    m_pos++;                    /* it's a status, increment  */

                e.set_status(status);           /* set the members in event  */
                RunningTime += Delta;           /* add in the time           */

                /*
                 * current time is ppqn according to the file, we have to
                 * adjust it to our own ppqn.  PPQN / ppqn gives us the
                 * ratio.
                 */

                CurrentTime = (RunningTime * c_ppqn) / ppqn;
                e.set_timestamp(CurrentTime);

                switch (status & 0xF0)   /* switch on the channelless status */
                {
                case EVENT_NOTE_OFF:     /* case for those with 2 data bytes */
                case EVENT_NOTE_ON:
                case EVENT_AFTERTOUCH:
                case EVENT_CONTROL_CHANGE:
                case EVENT_PITCH_WHEEL:

                    data[0] = read_byte();
                    data[1] = read_byte();

                    // some files have velocity == 0 as Note Off

                    if ((status & 0xF0) == EVENT_NOTE_ON && data[1] == 0)
                    {
                        e.set_status(EVENT_NOTE_OFF);
                    }
                    e.set_data(data[0], data[1]);         /* set data and add */
                    seq->add_event(&e);
                    seq->set_midi_channel(status & 0x0F); /* set midi channel */
                    break;

                case EVENT_PROGRAM_CHANGE:                /* one data item    */
                case EVENT_CHANNEL_PRESSURE:

                    data[0] = read_byte();
                    e.set_data(data[0]);                  /* set data and add */
                    seq->add_event(&e);
                    seq->set_midi_channel(status & 0x0F); /* set midi channel */
                    break;

                case 0xF0:              /* Meta MIDI events, should be FF !!  */

                    if (status == 0xFF)
                    {
                        type = read_byte();              /* get meta type     */
                        len = read_var();
                        switch (type)
                        {
                        case 0x7f:                       /* proprietary       */
                            if (len > 4)                 /* FF 7F len data    */
                            {
                                proprietary = read_long();
                                len -= 4;
                            }

                            if (proprietary == c_midibus)
                            {
                                seq->set_midi_bus(read_byte());
                                len--;
                            }
                            else if (proprietary == c_midich)
                            {
                                seq->set_midi_channel(read_byte());
                                len--;
                            }
                            else if (proprietary == c_timesig)
                            {
                                seq->set_bpm(read_byte());
                                seq->set_bw(read_byte());
                                len -= 2;
                            }
                            else if (proprietary == c_triggers)
                            {
                                int num_triggers = len / 4;
                                for (int i = 0; i < num_triggers; i += 2)
                                {
                                    unsigned long on = read_long();
                                    unsigned long length = (read_long() - on);
                                    len -= 8;
                                    seq->add_trigger(on, length, 0, false);
                                }
                            }
                            else if (proprietary == c_triggers_new)
                            {
                                int num_triggers = len / 12;
                                for (int i = 0; i < num_triggers; i++)
                                {
                                    unsigned long on = read_long();
                                    unsigned long off = read_long();
                                    unsigned long length = off - on + 1;
                                    unsigned long offset = read_long();
                                    len -= 12;
                                    seq->add_trigger(on, length, offset, false);
                                }
                            }

                            m_pos += len;                /* eat the rest      */
                            break;

                        case 0x2f:                       /* Trk Done          */
                            /*
                             * If delta is 0, then another event happened
                             * at the same time as the track end.  The
                             * sequence class discards the last note.  This
                             * fixes that.  A native Seq24 file will
                             * always have a Delta >= 1.
                             */

                            if (Delta == 0)
                                CurrentTime += 1;

                            seq->set_length(CurrentTime, false);
                            seq->zero_markers();
                            done = true;
                            break;

                        case 0x03:                        /* Track name      */
                            for (i = 0; i < len; i++)
                                TrackName[i] = read_byte();

                            TrackName[i] = '\0';
                            seq->set_name(TrackName);
                            break;

                        case 0x00:                        /* sequence number */
                            if (len == 0x00)
                                perf = 0;
                            else
                                perf = read_short();
                            break;

                        default:
                            for (i = 0; i < len; i++)
                                (void) read_byte();      /* ignore the rest */
                            break;
                        }
                    }
                    else if (status == 0xF0)
                    {
                        len = read_var();                /* sysex           */
                        m_pos += len;                    /* skip it         */
                        errprint("No support for SYSEX messages, discarding.");
                    }
                    else
                    {
                        errprintf("Unexpected system event : 0x%.2X", status);
                        return false;
                    }
                    break;

                default:

                    fprintf(stderr, "Unsupported MIDI event: %x\n", int(status));
                    return false;
                    break;
                }
            } /* while not done loading Trk chunk */

            /* Sequence has been filled, add it to the performance  */

            a_perf->add_sequence(seq, perf + (a_screen_set * c_seqs_in_set));
        }
        else   /* */
        {
            /*
             * We don't know what kind of chunk it is.  It's not a MTrk, we
             * don't know how to deal with it, so we just eat it.
             */

            fprintf(stderr, "Unsupported MIDI header detected: %8lX\n", ID);
            m_pos += TrackLength;
            done = true;
        }
    } /* for each track */

    /*
     * After all of the conventional MIDI tracks are read, we're now at
     * the "proprietary" Seq24 data section, which describes the various
     * features that Seq24 supports.  It consists of series of tags:
     *
     *  -   c_midictrl:
     *  -   c_midiclocks:
     *  -   c_notes:
     *  -   c_bpmtag:
     *  -   c_mutegroups:
     *
     *  The format is 1) tag ID; 2) length of data; 3) the data.
     */

    if ((file_size - m_pos) > (int) sizeof(unsigned long))
    {
        ID = read_long();                               /* Get ID + Length */
        if (ID == c_midictrl)
        {
            unsigned long seqs = read_long();
            for (unsigned int i = 0; i < seqs; i++)
            {
                a_perf->get_midi_control_toggle(i)->m_active = read_byte();
                a_perf->get_midi_control_toggle(i)->m_inverse_active =
                    read_byte();
                a_perf->get_midi_control_toggle(i)->m_status = read_byte();
                a_perf->get_midi_control_toggle(i)->m_data = read_byte();
                a_perf->get_midi_control_toggle(i)->m_min_value = read_byte();
                a_perf->get_midi_control_toggle(i)->m_max_value = read_byte();

                a_perf->get_midi_control_on(i)->m_active = read_byte();
                a_perf->get_midi_control_on(i)->m_inverse_active =
                    read_byte();
                a_perf->get_midi_control_on(i)->m_status = read_byte();
                a_perf->get_midi_control_on(i)->m_data = read_byte();
                a_perf->get_midi_control_on(i)->m_min_value = read_byte();
                a_perf->get_midi_control_on(i)->m_max_value = read_byte();

                a_perf->get_midi_control_off(i)->m_active = read_byte();
                a_perf->get_midi_control_off(i)->m_inverse_active =
                    read_byte();
                a_perf->get_midi_control_off(i)->m_status = read_byte();
                a_perf->get_midi_control_off(i)->m_data = read_byte();
                a_perf->get_midi_control_off(i)->m_min_value = read_byte();
                a_perf->get_midi_control_off(i)->m_max_value = read_byte();
            }
        }

        ID = read_long();                               /* Get ID + Length */
        if (ID == c_midiclocks)
        {
            /*
             * TrackLength is number of busses (?)
             */

            TrackLength = read_long();
            for (unsigned int x = 0; x < TrackLength; x++)
            {
                int bus_on = read_byte();
                a_perf->get_master_midi_bus()->set_clock(x, (clock_e) bus_on);
            }
        }
    }
    if ((file_size - m_pos) > (int) sizeof(unsigned long))
    {
        ID = read_long();                               /* Get ID + Length  */
        if (ID == c_notes)
        {
            unsigned int screen_sets = read_short();
            for (unsigned int x = 0; x < screen_sets; x++)
            {
                unsigned int len = read_short();        /* length of string */
                std::string notess;
                for (unsigned int i = 0; i < len; i++)
                    notess += read_byte();              /* unsigned!        */

                a_perf->set_screen_set_notepad(x, &notess);
            }
        }
    }

    if ((file_size - m_pos) > (int) sizeof(unsigned int))
    {
        ID = read_long();                               /* Get ID + Length  */
        if (ID == c_bpmtag)
        {
            long bpm = read_long();
            a_perf->set_bpm(bpm);
        }
    }

    /* Read in the mute group information. */

    if ((file_size - m_pos) > (int) sizeof(unsigned long))
    {
        ID = read_long();
        if (ID == c_mutegroups)
        {
            long length = read_long();
            if (c_gmute_tracks != length)
            {
                printf("corrupt data in mutegroup section\n");
            }
            for (int i = 0; i < c_seqs_in_set; i++)
            {
                a_perf->select_group_mute(read_long());
                for (int k = 0; k < c_seqs_in_set; ++k)
                {
                    a_perf->set_group_mute_state(k, read_long());
                }
            }
        }
    }

    /********** ADD NEW TAGS AT THE END OF THE LIST HERE **************/

    return true;
}

/**
 *  Writes 4 bytes, using the write_byte() function.
 *
 * \warning
 *      This code looks endian-dependent.
 */

void
midifile::write_long (unsigned long a_x)
{
    write_byte((a_x & 0xFF000000) >> 24);
    write_byte((a_x & 0x00FF0000) >> 16);
    write_byte((a_x & 0x0000FF00) >> 8);
    write_byte((a_x & 0x000000FF));
}

/**
 *  Writes 2 bytes, using the write_byte() function.
 *
 * \warning
 *      This code looks endian-dependent.
 */

void
midifile::write_short (unsigned short a_x)
{
    write_byte((a_x & 0xFF00) >> 8);
    write_byte((a_x & 0x00FF));
}

/**
 *  Writes 1 byte.  The byte is written to the m_char_list member, using a
 *  call to push_back().  We should inline this function.
 */

void
midifile::write_byte (unsigned char a_x)
{
    m_char_list.push_back(a_x);
}

/**
 *  Write the whole MIDI data and Seq24 information out to the file.
 */

bool midifile::write (perform * a_perf)
{
    int numtracks = 0;
    for (int i = 0; i < c_max_sequence; i++) /* get number of active tracks */
    {
        if (a_perf->is_active(i))
            numtracks++;
    }

    /*
     * Write the MIDI Format 1 header: 'MThd' and a length of 6.
     */

    write_long(0x4D546864);
    write_long(0x00000006);

    write_short(0x0001);                    /* format 1                 */
    write_short(numtracks);                 /* number of tracks         */
    write_short(c_ppqn);                    /* parts per quarter note   */

    /*
     * We should be good to load now for each Track in the midi file.
     */

    for (int curTrack = 0; curTrack < c_max_sequence; curTrack++)
    {
        if (a_perf->is_active(curTrack))
        {
            sequence * seq = a_perf->get_sequence(curTrack);
            std::list<char> l;
            seq->fill_list(&l, curTrack);

            write_long(0x4D54726B);         /* magic number 'MTrk'      */
            write_long(l.size());
            while (l.size() > 0)            /* write the track data     */
            {
                write_byte(l.back());
                l.pop_back();
            }
        }
    }

    write_long(c_midictrl);                 /* midi control tag         */
    write_long(0);
    write_long(c_midiclocks);               /* bus mute/unmute data     */
    write_long(0);
    write_long(c_notes);                    /* notepad data tag         */
    write_short(c_max_sets);
    for (int i = 0; i < c_max_sets; i++)
    {
        string * note = a_perf->get_screen_set_notepad(i);
        write_short(note->length());
        for (unsigned int j = 0; j < note->length(); j++)
            write_byte((*note)[j]);
    }

    write_long(c_bpmtag);                   /* bpm tag                  */
    write_long(a_perf->get_bpm());
    write_long(c_mutegroups);               /* the mute groups tag      */
    write_long(c_gmute_tracks);
    for (int j = 0; j < c_seqs_in_set; ++j)
    {
        a_perf->select_group_mute(j);
        write_long(j);
        for (int i = 0; i < c_seqs_in_set; ++i)
            write_long(a_perf->get_group_mute_state(i));
    }

    std::ofstream file(m_name.c_str(), ios::out | ios::binary | ios::trunc);
    if (! file.is_open())
        return false;

    char file_buffer[1024];                 /* enable bufferization     */
    file.rdbuf()->pubsetbuf(file_buffer, sizeof file_buffer);
    for
    (
        std::list<unsigned char>::iterator i = m_char_list.begin();
        i != m_char_list.end(); i++
    )
    {
        char c = *i;
        file.write(&c, 1);
    }
    m_char_list.clear();
    return true;
}

/*
 * midifile.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */

