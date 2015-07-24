Summary:	Loop based MIDI sequencer
Name:		seq24
Version:	0.9.2
Release:	1
License:	GPL
Group:		X11/Applications/Sound
Source0:	http://edge.launchpad.net/seq24/trunk/0.9.2/+download/%{name}-%{version}.tar.bz2
# Source0-md5:	ea0d28eb4991e9190dd10b43617b9f25
Patch0:		%{name}-jack-transport.patch
Patch1:		%{name}-lash-fix.patch
URL:		http://filter24.org/seq24/
BuildRequires:	alsa-lib-devel
BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:	gtkmm-devel
BuildRequires:	jack-audio-connection-kit-devel
BuildRequires:	lash-devel
BuildRequires:	libstdc++-devel
BuildRequires:	libtool
BuildRequires:	pkg-config
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Seq24 is a real-time midi sequencer. It was created to provide a very
simple interface for editing and playing MIDI 'loops'.

%prep
%setup -q
%patch0 -p0
%patch1 -p1

%build
%{__libtoolize}
%{__aclocal}
%{__autoheader}
%{__autoconf}
%{__automake}
%configure
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT{%{_desktopdir},%{_pixmapsdir}}

%{__make} install \
	DESTDIR=$RPM_BUILD_ROOT

cat > $RPM_BUILD_ROOT%{_desktopdir}/seq24.desktop <<EOF
[Desktop Entry]
Type=Application
Exec=seq24
Icon=seq24
Terminal=false
Name=Seq24
Comment=Minimal loop based midi sequencer
Categories=GTK;Audio;AudioVideo;Midi;Sequencer;
EOF

install src/pixmaps/seq24_32.xpm \
	$RPM_BUILD_ROOT%{_pixmapsdir}/seq24.xpm

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%doc AUTHORS ChangeLog README RTC SEQ24
%attr(755,root,root) %{_bindir}/*
%{_mandir}/man1/seq24.1*
%{_desktopdir}/*.desktop
%{_pixmapsdir}/*.xpm

