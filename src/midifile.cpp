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
 * \updates       2015-08-16
 * \license       GNU GPLv2 or above
 *
 */

#include <fstream>

#include "perform.h"                    // must precede midifile.h !
#include "midifile.h"
#include "sequence.h"

/**
 *  A manifest constant for controlling the length of a line-reading
 *  array in a configuration file.
 */

#define SEQ24_MIDI_LINE_MAX            1024

/**
 *  The maximum length of a Seq24 track name.  This is a bit excessive.
 */

#define TRACKNAME_MAX                   256

/**
 *  Principal constructor.
 *
 * \param a_name
 *      Provides the name of the MIDI file to be read or written.
 *
 * \param propformat
 *      If true, write out the MIDI file using the MIDI-compliant
 *      sequencer-specific prefix in from of the seq24-specific
 *      SeqSpec tags defined in the globals module.  This option is true
 *      by default.  Note that this option is only used in writing;
 *      reading can handle either format transparently.
 */

midifile::midifile (const std::string & a_name, bool propformat)
 :
    m_pos           (0),
    m_name          (a_name),
    m_data          (),
    m_char_list     (),
    m_new_format    (propformat)
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
 *      This code looks endian-dependent and integer-size dependent.
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
 *  Read a MIDI Variable-Length Value (VLV), which has a variable number
 *  of bytes.  This function reads the bytes while bit 7 is set in each
 *  byte.  Bit 7 is a continuation bit.  See write_varinum() for more
 *  information.
 */

unsigned long
midifile::read_varinum ()
{
    unsigned long result = 0;
    unsigned char c;
    while (((c = read_byte()) & 0x80) != 0x00)      /* while bit 7 is set  */
    {
        result <<= 7;                               /* shift result 7 bits */
        result += (c & 0x7F);                       /* add bits 0-6        */
    }
    result <<= 7;                                   /* bit was clear       */
    result += (c & 0x7F);
    return result;
}

/**
 *  This function opens a binary MIDI file and parses it into sequences
 *  and other application objects.
 */

bool
midifile::parse (perform * a_perf, int a_screen_set)
{
    bool result = true;
    std::ifstream file
    (
        m_name.c_str(), std::ios::in | std::ios::binary | std::ios::ate
    );
    if (! file.is_open())
    {
        errprintf("Error opening MIDI file '%s'", m_name.c_str());
        return false;
    }

    int file_size = file.tellg();                   /* get end offset ??  */
    file.seekg(0, std::ios::beg);                   /* seek to start      */
    try
    {
        m_data.resize(file_size);                   /* allocate more data */
    }
    catch (const std::bad_alloc & ex)
    {
        errprint("Memory allocation failed in midifile::parse()");
        return false;
    }
    file.read((char *) &m_data[0], file_size);
    file.close();

    unsigned long ID = read_long();                 /* read hdr chunk info  */
    unsigned long TrackLength = read_long();
    unsigned short Format = read_short();           /* 0,1,2                */
    unsigned short NumTracks = read_short();
    unsigned short ppqn = read_short();
    if (ID != 0x4D546864)                           /* magic number 'MThd' */
    {
        errprintf("Invalid MIDI header detected: %8lX\n", ID);
        return false;
    }
    if (Format != 1)                                /* only support format 1 */
    {
        errprintf("Unsupported MIDI format detected: %d\n", Format);
        return false;
    }

    /*
     * We should be good to load now, for each Track in the MIDI file.
     */

    event e;
    for (int curtrack = 0; curtrack < NumTracks; curtrack++)
    {
        unsigned long Delta;                        /* time                  */
        unsigned long RunningTime;
        unsigned long CurrentTime;
        char TrackName[TRACKNAME_MAX];              /* track name from file  */
        ID = read_long();                           /* Get ID + Length       */
        TrackLength = read_long();
        if (ID == 0x4D54726B)                       /* magic number 'MTrk'   */
        {
            unsigned short perf = 0;
            unsigned char status = 0;
            unsigned char type;
            unsigned char data[2];
            unsigned char laststatus;
            unsigned long proprietary = 0;          /* sequencer-specifics   */
            long len;
            sequence * seq = new sequence();
            bool done = false;                      /* done for each track   */
            if (seq == nullptr)
            {
                errprint("Memory allocation failed, midifile::parse()");
                return false;
            }
            seq->set_master_midi_bus(&a_perf->master_bus());    /* set buss  */
            RunningTime = 0;                    /* reset time                */
            while (! done)                      /* get each event in the Trk */
            {
                Delta = read_varinum();             /* get time delta            */
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
                switch (status & 0xF0)   /* switch on channel-less status    */
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
                        len = read_varinum();
                        switch (type)
                        {
                        case 0x7F:                       /* proprietary       */
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

                        case 0x2F:                       /* Trk Done          */

                            /*
                             * If delta is 0, then another event happened
                             * at the same time as the track-end.  The
                             * sequence class discards the last note.
                             * This fixes that.  A native Seq24 file will
                             * always have a Delta >= 1.
                             */

                            if (Delta == 0)
                                CurrentTime += 1;

                            seq->set_length(CurrentTime, false);
                            seq->zero_markers();
                            done = true;
                            break;

                        case 0x03:                        /* Track name      */
                            if (len > TRACKNAME_MAX)
                                len = TRACKNAME_MAX;      /* avoid a vuln    */

                            for (int i = 0; i < len; i++)
                                TrackName[i] = read_byte();

                            TrackName[len] = '\0';
                            seq->set_name(TrackName);
                            break;

                        case 0x00:                        /* sequence number */
                            perf = (len == 0x00) ? 0 : read_short();
                            break;

                        default:
                            for (int i = 0; i < len; i++)
                                (void) read_byte();      /* ignore the rest */
                            break;
                        }
                    }
                    else if (status == 0xF0)
                    {
                        len = read_varinum();                /* sysex           */
                        m_pos += len;                    /* skip it         */
                        errprint("No support for SYSEX messages, skipping...");
                    }
                    else
                    {
                        errprintf("Unexpected System event: 0x%.2X", status);
                        return false;
                    }
                    break;

                default:

                    errprintf("Unsupported MIDI event: %x\n", int(status));
                    return false;
                    break;
                }
            }                          /* while not done loading Trk chunk */

            /* Sequence has been filled, add it to the performance  */

            a_perf->add_sequence(seq, perf + (a_screen_set * c_seqs_in_set));
        }
        else
        {
            /*
             * We don't know what kind of chunk it is.  It's not a MTrk, we
             * don't know how to deal with it, so we just eat it.
             */

            if (curtrack > 0)                          /* MThd comes first */
                errprintf("Unsupported MIDI chunk, skipping: %8lX\n", ID);

            m_pos += TrackLength;
        }
    }                                                   /* for each track  */
    if (result)
        result = parse_proprietary(a_perf, file_size);

    return result;
}

/**
 *  Parse the proprietary header, figuring out if it is the new format, or
 *  the legacy format.
 *
 *  For convenience, this function also checks the amount of file data
 *  left.
 *
 * \param file_size
 *      The size of the data file.  This value is compared against the
 *      member m_pos (the position inside m_data[]), to make sure there is
 *      enough data left to process.
 *
 * \return
 *      Returns the control-tag value found.  These are the values, such as
 *      c_midich, found in the globals module, that indicate the type of
 *      sequencer-specific data that comes next.  If there is not enough
 *      data to process, then 0 is returned.
 */

unsigned long
midifile::parse_prop_header (int file_size)
{
    unsigned long result = 0;
    if ((file_size - m_pos) > (int) sizeof(unsigned long))
    {
        result = read_long();
        unsigned char status = (result & 0xFF000000) >> 24;
        if (status == 0xFF)
        {
            m_pos -= 3;                         /* back up to retrench      */
            unsigned char type = read_byte();   /* get meta type            */
            if (type == 0x7F)
            {
                /* long len = */ (void) read_varinum();
                result = read_long();
            }
            else
            {
                errprint("Bad status in proprietary file footer");
                return false;
            }
        }
    }
    return result;
}

/**
 *  After all of the conventional MIDI tracks are read, we're now at the
 *  "proprietary" Seq24 data section, which describes the various features
 *  that Seq24 supports.  It consists of series of tags:
 *
 *  -   c_midictrl:
 *  -   c_midiclocks:
 *  -   c_notes:
 *  -   c_bpmtag:
 *  -   c_mutegroups:
 *
 *  (There are more tags defined in the globals module, but they are not
 *  used in this function.)
 *
 *  The format is 1) tag ID; 2) length of data; 3) the data.
 *
 * \change ca 2015-08-16
 *      First, we separate out this function for a little more clarify.
 *      Then we add code to handle reading both the legacy Seq24 format
 *      and the new, MIDI-compliant format.  Note that the format is not
 *      quite correct, since it doesn't handle a MIDI manufacturer's ID,
 *      making it a single byte that is part of the data.
 *
 * \param a_perf
 *      The performance object that is being set via the incoming MIDI
 *      file.
 *
 * \param file_size
 *      The file size as determined in the parse() function.
 *
 *  There is also an implicit parameter in the m_pos member variable.
 */

bool
midifile::parse_proprietary (perform * a_perf, int file_size)
{
    bool result = true;
    unsigned long proprietary = parse_prop_header(file_size);
    if (proprietary == c_midictrl)
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
            a_perf->get_midi_control_on(i)->m_inverse_active = read_byte();
            a_perf->get_midi_control_on(i)->m_status = read_byte();
            a_perf->get_midi_control_on(i)->m_data = read_byte();
            a_perf->get_midi_control_on(i)->m_min_value = read_byte();
            a_perf->get_midi_control_on(i)->m_max_value = read_byte();
            a_perf->get_midi_control_off(i)->m_active = read_byte();
            a_perf->get_midi_control_off(i)->m_inverse_active = read_byte();
            a_perf->get_midi_control_off(i)->m_status = read_byte();
            a_perf->get_midi_control_off(i)->m_data = read_byte();
            a_perf->get_midi_control_off(i)->m_min_value = read_byte();
            a_perf->get_midi_control_off(i)->m_max_value = read_byte();
        }
    }
    proprietary = parse_prop_header(file_size);
    if (proprietary == c_midiclocks)
    {
        unsigned long busscount = read_long();
        for (unsigned int buss = 0; buss < busscount; buss++)
        {
            int clocktype = read_byte();
            a_perf->master_bus().set_clock(buss, (clock_e) clocktype);
        }
    }
    proprietary = parse_prop_header(file_size);
    if (proprietary == c_notes)
    {
        unsigned int screen_sets = read_short();
        for (unsigned int x = 0; x < screen_sets; x++)
        {
            unsigned int len = read_short();            /* length of string */
            std::string notess;
            for (unsigned int i = 0; i < len; i++)
                notess += read_byte();                  /* unsigned!        */

            a_perf->set_screen_set_notepad(x, &notess);
        }
    }
    proprietary = parse_prop_header(file_size);
    if (proprietary == c_bpmtag)
    {
        long bpm = read_long();
        a_perf->set_bpm(bpm);
    }

    /* Read in the mute group information. */

    proprietary = parse_prop_header(file_size);
    if (proprietary == c_mutegroups)
    {
        long length = read_long();
        if (c_gmute_tracks != length)
        {
            errprint("corrupt data in mute-group section");
            result = false;                         /* but keep going */
        }
        for (int i = 0; i < c_seqs_in_set; i++)
        {
            long groupmute = read_long();
            a_perf->select_group_mute(groupmute);
#ifdef PLATFORM_DEBUG_XXX
            if (groupmute != 0)
                fprintf(stderr, "group-mute[%d] = %ld\n", i, groupmute);
#endif
            for (int k = 0; k < c_seqs_in_set; ++k)
            {
                long gmutestate = read_long();
                a_perf->set_group_mute_state(k, gmutestate);
#ifdef PLATFORM_DEBUG_XXX
                if (gmutestate != 0)
                {
                    fprintf
                    (
                        stderr, "group-mute-state[%d] = %ld\n",
                        k, gmutestate
                    );
                }
#endif
            }
        }
    }

    /*
     * ADD NEW TAGS AT THE END OF THE LIST HERE.
     */

    return result;
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
 *  Writes a MIDI Variable-Length Value (VLV), which has a variable number
 *  of bytes.
 *
 *  A MIDI file Variable Length Value is stored in bytes. Each byte has
 *  two parts: 7 bits of data and 1 continuation bit. The highest-order
 *  bit is set to 1 if there is another byte of the number to follow. The
 *  highest-order bit is set to 0 if this byte is the last byte in the
 *  VLV.
 *
 *  To recreate a number represented by a VLV, first you remove the
 *  continuation bit and then concatenate the leftover bits into a single
 *  number.
 *
 *  To generate a VLV from a given number, break the number up into 7 bit
 *  units and then apply the correct continuation bit to each byte.
 *
 *  In theory, you could have a very long VLV number which was quite
 *  large; however, in the standard MIDI file specification, the maximum
 *  length of a VLV value is 5 bytes, and the number it represents can not
 *  be larger than 4 bytes.
 *
 *  Here are some common cases:
 *
 *      -  Numbers between 0 and 127 (0x7F) are represented by a single
 *         byte.
 *      -  0x80 is represented as "0x81 0x00".
 *      -  0x0FFFFFFF (the largest number) is represented as "0xFF 0xFF
 *         0xFF 0x7F".

 */

void
midifile::write_varinum (unsigned long value)
{
   unsigned long buffer = value & 0x7f;
   while ((value >>= 7) > 0)
   {
       buffer <<= 8;
       buffer |= 0x80;
       buffer += (value & 0x7f);
   }
   for (;;)
   {
      write_byte((unsigned char) (buffer & 0xff));
      if (buffer & 0x80)                            /* continuation bit?    */
         buffer >>= 8;                              /* yes                  */
      else
         break;                                     /* no, we are done      */
   }
}

/**
 *  Writes a "proprietary" Seq24 footer header in either the new
 *  MIDI-compliant format, or the legacy Seq24 format.  This function does
 *  not write the data.  It replaces calls such as "write_long(c_midich)"
 *  in the proprietary secton of write().
 *
 *  The legacy format just writes the control tag (0x242400xx).  The new
 *  format writes 0xFF 0x7F len 0x242400xx.
 *
 * \warning
 *      Currently, the manufacturer ID is not handled; it is part of the
 *      data, which can be misleading in programs that analyze MIDI files.
 *
 * \param control_tag
 *      Determines the type of sequencer-specific section to be written.
 *      It should be one of the value in the globals module, such as
 *      c_midibus or c_mutegroups.
 *
 * \param data_length
 *      The amount of data that will be written.  This parameter does not
 *      count the length of the header itself.
 */

void
midifile::write_prop_header
(
    unsigned long control_tag,
    long data_length
)
{
    if (! global_legacy_format)
    {
        int len = data_length + 4;         /* sizeof(control_tag);          */
        write_byte(0xFF);
        write_byte(0x7F);
        write_varinum(len);
    }
    write_long(control_tag);                /* use legacy output call       */
}


/**
 *  Write the whole MIDI data and Seq24 information out to the file.
 */

bool
midifile::write (perform * a_perf)
{
    int numtracks = 0;
    for (int i = 0; i < c_max_sequence; i++) /* get number of active tracks */
    {
        if (a_perf->is_active(i))
            numtracks++;
    }
    write_long(0x4D546864);                 /* MIDI Format 1 header MThd    */
    write_long(6);                          /* Length of the header         */
    write_short(1);                         /* MIDI Format 1                */
    write_short(numtracks);                 /* number of tracks             */
    write_short(c_ppqn);                    /* parts per quarter note       */

    /*
     * Write out the active tracks.  The value of c_max_sequence is 1024.
     */

    for (int curtrack = 0; curtrack < c_max_sequence; curtrack++)
    {
        if (a_perf->is_active(curtrack))
        {
            sequence * seq = a_perf->get_sequence(curtrack);
            std::list<char> l;              /* should be unsigned char! */
            seq->fill_list(&l, curtrack);
            write_long(0x4D54726B);         /* magic number 'MTrk'      */
            write_long(l.size());
            while (l.size() > 0)            /* write the track data     */
            {
                /*
                 * \warning
                 *      This writing backwards reverses the order of some
                 *      events that are otherwise equivalent in time-stamp
                 *      and rank.  But it must be done this way, otherwise
                 *      many items are backward.
                 */

                 write_byte(l.back());
                 l.pop_back();
            }
        }
    }

    /*
     * Write out the proprietary section, using the new format if the
     * legacy format is not in force.
     */

    write_prop_header(c_midictrl, 4);           /* midi control tag + 4     */
    write_long(0);
    write_prop_header(c_midiclocks, 4);         /* bus mute/unmute data + 4 */
    write_long(0);

    int datasize = 2;                           /* first value is short     */
    for (int i = 0; i < c_max_sets; i++)
    {
        std::string * note = a_perf->get_screen_set_notepad(i);
        datasize += 2 + note->length();         /* short + note length      */
    }
    write_prop_header(c_notes, datasize);       /* notepad data tag + data  */
    write_short(c_max_sets);                    /* data, not a tag          */
    for (int i = 0; i < c_max_sets; i++)
    {
        std::string * note = a_perf->get_screen_set_notepad(i);
        write_short(note->length());
        for (unsigned int j = 0; j < note->length(); j++)
            write_byte((*note)[j]);
    }
    write_prop_header(c_bpmtag, 4);             /* bpm tag + 4              */
    write_long(a_perf->get_bpm());
    write_prop_header(c_mutegroups, 4);         /* the mute groups tag + 4  */
    write_long(c_gmute_tracks);                 /* data, not a tag          */

    /*
     * We need a way to make this optional.  Why write 4096 bytes
     * needlessly?
     */

    for (int j = 0; j < c_seqs_in_set; ++j)
    {
        a_perf->select_group_mute(j);
        write_long(j);
        for (int i = 0; i < c_seqs_in_set; ++i)
            write_long(a_perf->get_group_mute_state(i));
    }

    std::ofstream file
    (
        m_name.c_str(), std::ios::out | std::ios::binary | std::ios::trunc
    );
    if (! file.is_open())
        return false;

    char file_buffer[SEQ24_MIDI_LINE_MAX];  /* enable bufferization     */
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
