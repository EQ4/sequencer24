Name:       seq24
Summary:    Minimalistic, loop-based MIDI sequencer
Version:    0.9.2
Release:    4

Source:     http://edge.launchpad.net/%{name}/trunk/%{version}/+download/%{name}-%{version}.tar.bz2
Patch0:     seq24-fix-jacksession.patch
URL:        https://edge.launchpad.net/%{name}/
License:    GPLv2+
Group:      Sound
BuildRoot:  %{_tmppath}/%{name}-buildroot

BuildRequires:  libalsa-devel
BuildRequires:  gtkmm2.4-devel
BuildRequires:  jackit-devel

%description
Seq24 is a real-time midi sequencer. It was created to provide a very simple
interface for editing and playing midi 'loops'. After searching for a software
based sequencer that would provide the functionality needed for a live techno
performance, such as the Akai MPC line, the Kawai Q80 sequencer, or the
popular Alesis MMT-8, I found nothing similar in the software realm. I set out
to create a very minimal sequencer that excludes the bloated features of the
large software sequencers, and includes a small subset of features that I have
found usable in performing.

%prep
%setup -q
%patch0 -p0

%build
%configure
%make

%install
rm -rf %{buildroot}
%makeinstall
mkdir -p %{buildroot}%{_datadir}/pixmaps
cp src/pixmaps/%{name}.xpm %{buildroot}%{_datadir}/pixmaps
#menu

mkdir -p %{buildroot}%{_datadir}/applications
cat > %{buildroot}%{_datadir}/applications/mandriva-%{name}.desktop << EOF
[Desktop Entry]
Name=Seq24
Comment=Loop-based MIDI sequencer
Exec=%{_bindir}/%{name}
Icon=%{name}
Terminal=false
Type=Application
StartupNotify=true
Categories=GTK;AudioVideo;Audio;X-MandrivaLinux-Multimedia-Sound;
EOF

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc README AUTHORS ChangeLog COPYING RTC SEQ24
%{_bindir}/%{name}
%{_mandir}/man1/%{name}*
%{_datadir}/applications/mandriva-%{name}.desktop
%{_datadir}/pixmaps/%{name}.xpm


%changelog
* Fri Dec 23 2011 Frank Kober <emuse@mandriva.org> 0.9.2-3
+ Revision: 744872
- fix broken jacksession with patch

* Fri Dec 23 2011 Frank Kober <emuse@mandriva.org> 0.9.2-2
+ Revision: 744727
- rebuild to include JACK Session support from jack 1.9.8
  o remove find_lang (package has no translations)
  o cleanup spec

* Sun Nov 28 2010 Frank Kober <emuse@mandriva.org> 0.9.2-1mdv2011.0
+ Revision: 602302
- new version 0.9.2

* Mon Nov 08 2010 Frank Kober <emuse@mandriva.org> 0.9.1-1mdv2011.0
+ Revision: 595128
- new version 0.9.1
  o drop patch0

* Sun Mar 21 2010 Frank Kober <emuse@mandriva.org> 0.9.0-3mdv2010.1
+ Revision: 526009
- install desktop icon

* Wed Mar 03 2010 Frank Kober <emuse@mandriva.org> 0.9.0-2mdv2010.1
+ Revision: 513839
- apply patch porting to bzr rev 70

* Wed Dec 16 2009 Jérôme Brenier <incubusss@mandriva.org> 0.9.0-1mdv2010.1
+ Revision: 479528
- new version 0.9.0
- fix source url
- fix license tag
- remove no more needed workaround in %%build
- fix desktop file
- fix files list

  + Thierry Vignaud <tv@mandriva.org>
    - rebuild
    - rebuild
    - drop old menu
    - kill re-definition of %%buildroot on Pixel's request
    - kill desktop-file-validate's 'warning: key "Encoding" in group "Desktop Entry" is deprecated'

  + Pixel <pixel@mandriva.com>
    - rpm filetriggers deprecates update_menus/update_scrollkeeper/update_mime_database/update_icon_cache/update_desktop_database/post_install_gconf_schemas

  + Austin Acton <austin@mandriva.org>
    - sync
    - new version

  + Olivier Blin <blino@mandriva.org>
    - restore BuildRoot

* Wed Jun 13 2007 Austin Acton <austin@mandriva.org> 0.8.6-2mdv2008.0
+ Revision: 38926
- rebuild to close bug #26892
- Import seq24



* Wed Jun 28 2006 Guillaume Bedot <littletux@mandriva.org> 0.8.6-1mdv2007.0
- 0.8.6
- add dump binary
- fix buildrequires
- xdg menu
- remove rpath

* Mon Oct 10 2005 Nicolas L�cureuil <neoclust@mandriva.org> 0.7.0-2mdk
- Fix BuildRequires
- %%mkrel 

* Sun Oct 09 2005 Austin Acton <austin@mandriva.org> 0.7.0-1mdk
- New release 0.7.0

* Thu Aug 25 2005 Austin Acton <austin@mandriva.org> 0.6.3-1mdk
- New release 0.6.3

* Fri Feb 26 2005 Austin Acton <austin@mandrake.org> 0.6.1-1mdk
- 0.6.1
- source URL

* Thu Jan 13 2005 Austin Acton <austin@mandrake.org> 0.6.0-1mdk
- 0.6.0

* Tue Jul 13 2004 Austin Acton <austin@mandrake.org> 0.5.1-1mdk
- 0.5.1

* Wed Jun 16 2004 Austin Acton <austin@mandrake.org> 0.5.0-1mdk
- 0.5.0
- configure 2.5

* Mon Feb 16 2004 Austin Acton <austin@mandrake.org> 0.4.4-1mdk
- 0.4.4

* Wed May 14 2003 Austin Acton <aacton@yorku.ca> 0.4.2-1mdk
- initial package
