Summary:	Real-time MIDI sequencer
Summary(pl.UTF-8):	Sekwencer MIDI działający w czasie rzeczywistym
Name:		seq24
Version:	0.8.7
Release:	1
License:	GPL
Group:		X11/Applications/Sound
Source0:	http://filter24.org/seq24/%{name}-%{version}.tar.gz
# Source0-md5:	b2c39dad73f803727c39c2e2e443ebdb
Source1:	%{name}.desktop
Patch0:		sigc22-fix.patch
URL:		http://filter24.org/seq24/
BuildRequires:	alsa-lib-devel >= 0.9.0
BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:	gtkmm-devel >= 2.4.0
BuildRequires:	jack-audio-connection-kit-devel >= 0.90.0
BuildRequires:	lash-devel
BuildRequires:	libstdc++-devel
BuildRequires:	libtool
BuildRequires:	pkgconfig
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Seq24 is a real-time midi sequencer. It was created to provide a very
simple interface for editing and playing MIDI 'loops'.

%description -l pl.UTF-8
Seq24 jest sekwencerem działającym w czasie rzeczywistym. Został
stworzony z myślą o prostocie interfejsu do edycji i odtwarzania
'pętli' MIDI.

%prep
%setup -q
%patch0 -p1

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
install -d $RPM_BUILD_ROOT%{_desktopdir}

%{__make} install \
	DESTDIR=$RPM_BUILD_ROOT

install %{SOURCE1} $RPM_BUILD_ROOT%{_desktopdir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%doc AUTHORS ChangeLog README RTC SEQ24
%attr(755,root,root) %{_bindir}/*
%{_desktopdir}/*.desktop
