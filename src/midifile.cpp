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
 * \updates       2015-09-04
 * \license       GNU GPLv2 or above
 *
 *  For a quick guide to the MIDI format, see, for example:
 *
 *  http://www.mobilefish.com/tutorials/midi/midi_quickguide_specification.html
 *
 *  It is important to note that most sequencers have taken a shortcut or
 *  two in reading the MIDI format.  For example, most will silently
 *  ignored an unadorned control tag (0x242400nn) which has not been
 *  package up as a proper sequencer-specific meta event.  The midicvt
 *  program (https://github.com/ahlstromcj/midicvt, derived from midicomp,
 *  midi2text, and mf2t/t2mf) does not ignore this lack, and hence we
 *  decided to provide a new, more strict input and output format for the
 *  the proprietary track in Sequencer24.
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
 *  The chunk header value for the Sequencer24 proprietary section.
 *  We will try other chunks, as well, since, as per the MIDI
 *  specification, unknown chunks should not cause an error in a sequencer
 *  (or our midicvt program).
 */

#define PROPRIETARY_CHUNK_TAG          0x4D54726B       /* "MTrk"           */

/**
 *  Provides the sequence number for the proprietary data when using the new
 *  format.  (There is no track-name for the legacy format.)  Can't use
 *  numbers, such as 0xFFFF, that have MIDI meta tags in them, confuses
 *  our proprietary track parser.
 */

#define PROPRIETARY_SEQ_NUMBER         0x7777           /* high enough?     */

/**
 *  Provides the track name for the proprietary data when using the new
 *  format.  (There is no track-name for the legacy format.)
 */

#define PROPRIETARY_TRACK_NAME         "Sequencer24-S"

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
     * Note that NumTracks doesn't count the Seq24 proprietary footer
     * section, even if it uses the new format, so that section will still
     * be read properly after all normal tracks have been processed.
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
                Delta = read_varinum();         /* get time delta            */
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

                    /* some files have velocity == 0 as Note Off */

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
            {
                errprintf("Unsupported MIDI chunk, skipping: %8lX\n", ID);
            }
            m_pos += TrackLength;
        }
    }                                                   /* for each track  */
    if (result)
        result = parse_proprietary_track(a_perf, file_size);

    return result;
}

/**
 *  Parse the proprietary header, figuring out if it is the new format, or
 *  the legacy format, for sequencer-specific data.
 *
 *  The new format creates a final track chunk, starting with "MTrk".
 *  Then comes the delta-time (here, 0), and the event.  An event is a
 *  MIDI event, a SysEx event, or a Meta event.
 *
 *  A MIDI Sequencer Specific meta message includes either a delta time or
 *  absolute time, and the MIDI Sequencer Specific event encoded as
 *  follows:
 *
\verbatim
        0xFF 0x7F 0x02 length data
\endverbatim
 *
 *  For convenience, this function first checks the amount of file data
 *  left.  Then it reads a long value.  If the value starts with FF, then
 *  that signals the new format.  Otherwise, it is probably the old
 *  format, and the long value is a control tag (0x242400nn), which can be
 *  returned immedidately.
 *
 *  If it is the new format, we back up to the FF, then get the next byte,
 *  which should be a 7F.  If so, then we read the length (a variable
 *  length value) of the data, and then read the long value, which should
 *  be the control tag, which, again, is returned by this function.
 *
 * \note
 *      Most sequencers seem to be tolerant of both the lack of an "MTrk"
 *      marker and of the presence of an unwrapped control tag, and so can
 *      handle both the old and new formats of the final proprietary track.
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
        result = read_long();                   /* status (new), or tag     */
        unsigned char status = (result & 0xFF000000) >> 24;
        if (status == 0xFF)
        {
            m_pos -= 3;                         /* back up to retrench      */
            unsigned char type = read_byte();   /* get meta type            */
            if (type == 0x7F)
            {
                (void) read_varinum();          /* prop section length      */
                result = read_long();           /* control tag              */
            }
            else
            {
                fprintf
                (
                    stderr,
                    "Bad status '%x' in proprietary section near offset %x",
                    int(type), m_pos
                );
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
 *      -   c_midictrl
 *      -   c_midiclocks
 *      -   c_notes
 *      -   c_bpmtag
 *      -   c_mutegroups
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
midifile::parse_proprietary_track (perform * a_perf, int file_size)
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
 *
 *  Also see the varinum_size() function.
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
 *  Calculates the length of a variable length value.  This function is
 *  needed when calculating the length of a track.  Note that it handles
 *  only the following situations:
 *
 *  https://en.wikipedia.org/wiki/Variable-length_quantity
 *
\verbatim
       1 byte:  0x00 to 0x7F
       2 bytes: 0x80 to 0x3FFF
       3 bytes: 0x4000 to 0x001FFFFF
       4 bytes: 0x200000 to 0x0FFFFFFF
\endverbatim
 *
 * \return
 *      Returns values as noted above.  Anything beyond that range returns
 *      0.
 */

long
midifile::varinum_size (long len) const
{
    int result = 0;
    if (len >= 0x00 && len < 0x80)
        result = 1;
    else if (len >= 0x80 && len < 0x4000)
        result = 2;
    else if (len >= 0x4000 && len < 0x200000)
        result = 3;
    else if (len >= 0x200000 && len < 0x10000000)
        result = 4;

    return result;
}

/**
 * We want to write:
 *
 *  -   0x4D54726B.
 *       The track tag "MTrk".  The MIDI spec requires that
 *      software can skip over non-standard chunks. "Prop"?  Would
 *      require a fix to midicvt.
 *  -   0xaabbccdd.
 *      The length of the track.  This needs to be
 *      calculated somehow.
 *  -   0x00.  A zero delta time.
 *  -   0x7f7f, The sequence number, a special value, well out of
 *      our normal range.
 *  -   The name of the track:
 *      -   "Seq24-Spec"
 *      -   "Sequencer24-S"
 *
 *   Then follows the proprietary data, written in the normal manner.
 *
 *   Finally, tack on the track-end meta-event.
 *
 *      Components of final track size:
 *
 *          -# Delta time.  1 byte, always 0x00.
 *          -# Sequence number.  5 bytes.  OPTIONAL.  We won't write it.
 *          -# Track name. 3 + 10 or 3 + 15
 *          -# Series of proprietary specs:
 *             -# Prop header:
 *                -# If legacy format, 4 bytes.
 *                -# Otherwise, 2 bytes + varinum_size(length) + 4 bytes.
 *                -# Length of the prop data.
 *          -# Track End. 3 bytes.
 */

/**
 *  Writes a "proprietary" Seq24 footer header in either the new
 *  MIDI-compliant format, or the legacy Seq24 format.  This function does
 *  not write the data.  It replaces calls such as "write_long(c_midich)"
 *  in the proprietary secton of write().
 *
 *  The legacy format just writes the control tag (0x242400xx).  The new
 *  format writes 0x00 0xFF 0x7F len 0x242400xx; the first 0x00 is the
 *  delta time.
 *
 *  In the new format, the 0x24 is a kind of "manufacturer ID".
 *  At http://www.midi.org/techspecs/manid.php we see that most
 *  manufacturer IDs start with 0x00, and are thus three bytes long,
 *  or start with codes at 0x40 and above.  Similary,
 *  http://sequence15.blogspot.com/2008/12/midi-manufacturer-ids.html
 *  shows that no manufacturer uses 0x24.
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
    if (m_new_format)
    {
        int len = data_length + 4;          /* data + sizeof(control_tag);  */
        write_byte(0x00);                   /* delta time                   */
        write_byte(0xFF);
        write_byte(0x7F);
        write_varinum(len);
    }
    write_long(control_tag);                /* use legacy output call       */
}

/**
 *  Calculates the size of a proprietary item, as written by the
 *  write_prop_header() function, plus whatever is called to write the data.
 *  If using the new format, the length includes the sum of sequencer-specific
 *  tag (0xFF 0x7F) and the size of the variable-length value.  Then, for
 *  legacy and new format, 4 bytes are added for the Seq24 MIDI control
 *  value, and the the data length is added.
 */

long
midifile::prop_item_size (long data_length) const
{
    long result = 0;
    if (m_new_format)
    {
        int len = data_length + 4;          /* data + sizeof(control_tag);  */
        result += 3;                        /* count delta time, meta bytes */
        result += varinum_size(len);        /* count the length bytes       */
    }
    result += 4;                            /* write_long(control_tag);     */
    result += data_length;                  /* add the data size itself     */
    return result;
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
    (void) write_proprietary_track(a_perf);

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

/**
 *  Writes out the proprietary section, using the new format if the
 *  legacy format is not in force.
 *
 *  The first thing to do, for the new format only, is calculate the length
 *  of this big section of data.  This was quite tricky; we tweaked and
 *  adjusted until the midicvt program handled the whole new-format file
 *  without emitting any errors.
 */

bool
midifile::write_proprietary_track (perform * a_perf)
{
    long tracklength = 0;
    int cnotesz = 2;                            /* first value is short     */
    for (int i = 0; i < c_max_sets; i++)
    {
        std::string * note = a_perf->get_screen_set_notepad(i);
        cnotesz += 2 + note->length();          /* short + note length      */
    }

    /*
     * We need a way to make the group mute data optional.  Why write 4096
     * bytes of zeroes?
     */

    int gmutesz = 4 + c_seqs_in_set * (4 + c_seqs_in_set * 4);
    if (m_new_format)                           /* calculate track size     */
    {
        tracklength += seq_number_size();       /* bogus sequence number    */
        tracklength += track_name_size(PROPRIETARY_TRACK_NAME);
        tracklength += prop_item_size(0);       /* c_midictrl               */
        tracklength += prop_item_size(0);       /* c_midiclocks             */
        tracklength += prop_item_size(cnotesz); /* c_notes                  */
        tracklength += prop_item_size(4);       /* c_bpmtag                 */
        tracklength += prop_item_size(gmutesz); /* c_mutegroups             */
        tracklength += track_end_size();        /* Meta TrkEnd              */
    }
    if (m_new_format)                           /* write beginning of track */
    {
        write_long(PROPRIETARY_CHUNK_TAG);      /* "MTrk" or something else */
        write_long(tracklength);
        write_seq_number(PROPRIETARY_SEQ_NUMBER); /* bogus sequence number   */
        write_track_name(PROPRIETARY_TRACK_NAME);
    }
    write_prop_header(c_midictrl, 0);           /* midi control tag + 4     */
    write_prop_header(c_midiclocks, 0);         /* bus mute/unmute data + 4 */
    write_prop_header(c_notes, cnotesz);        /* notepad data tag + data  */
    write_short(c_max_sets);                    /* data, not a tag          */
    for (int i = 0; i < c_max_sets; i++)        /* see "cnotesz" calc       */
    {
        std::string * note = a_perf->get_screen_set_notepad(i);
        write_short(note->length());
        for (unsigned int j = 0; j < note->length(); j++)
            write_byte((*note)[j]);
    }
    write_prop_header(c_bpmtag, 4);             /* bpm tag + long data      */
    write_long(a_perf->get_bpm());              /* 4 bytes                  */
    write_prop_header(c_mutegroups, gmutesz);   /* the mute groups tag etc. */
    write_long(c_gmute_tracks);                 /* data, not a tag          */
    for (int j = 0; j < c_seqs_in_set; ++j)     /* should be optional!      */
    {
        a_perf->select_group_mute(j);
        write_long(j);
        for (int i = 0; i < c_seqs_in_set; ++i)
            write_long(a_perf->get_group_mute_state(i));
    }
    if (m_new_format)
    {
        write_track_end();
    }
    return true;
}

/**
 *  Writes out a track name.  Note that we have to precede this "event"
 *  with a delta time value, set to 0.
 */

void
midifile::write_track_name (const std::string & trackname)
{
    bool ok = ! trackname.empty();
    if (ok)
    {
        write_byte(0x00);                               /* delta time       */
        write_byte(0xFF);                               /* meta tag         */
        write_byte(0x03);                               /* second byte      */
        write_varinum((unsigned long) trackname.size());
        for (int i = 0; i < int(trackname.size()); i++)
            write_byte(trackname[i]);
    }
}

/**
 *  Calculates the size of a trackname and the meta event that specifies
 *  it.
 */

long
midifile::track_name_size (const std::string & trackname) const
{
    long result = 0;
    if (! trackname.empty())
    {
        result += 3;                                    /* 0x00 0xFF 0x03   */
        result += varinum_size(long(trackname.size())); /* variable length  */
        result += long(trackname.size());               /* data size        */
    }
    return result;
}

/**
 *  Writes out a sequence number.  The format is "FF 00 02 ss ss", where
 *  "02" is actually the constant length of the data.  We have to precede
 *  these values with a 0 delta time, of course.
 */

void
midifile::write_seq_number (unsigned short seqnum)
{
    write_byte(0x00);                           /* delta time               */
    write_byte(0xFF);                           /* meta tag                 */
    write_byte(0x00);                           /* second byte              */
    write_byte(0x02);                           /* finish sequence tag      */
    write_short(seqnum);                        /* write sequence number    */
}

/**
 *  Writes out the end-of-track marker.
 */

void
midifile::write_track_end ()
{
    write_byte(0xFF);                       /* meta tag                     */
    write_byte(0x2F);
    write_byte(0x00);
}

/*
 * midifile.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
