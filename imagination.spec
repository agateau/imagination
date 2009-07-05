Summary:        DVD to MPEG-4 converter
Name:           imagination
Version:        1.0
Release:        1.%{?dist}
License:        GPL
Group:          Video
URL:            http://imagination.sourceforge.net
Source0:        %{name}-%version.tar.gz
BuildRoot:      %{_tmppath}/%name-%version-buildroot
BuildRequires:	glib2-devel, gtk2-devel
Requires:	glib2, gtk2
Requires:	ffmpeg >= 0.4.9

%description
Imagination is a DVD-slideshow application built with the GTK+2 toolkit.
It allows you to create a slideshow of pictures to be viewed on a home DVD player.

%prep
%setup -q -n %{name}-%{version}

%build
%configure

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=%{buildroot} install

# remove all wanted files
rm -f %{buildroot}%{_libdir}/%{name}/*.la
rm -f %{buildroot}%{_datadir}/%{name}/pixmaps/Makefile*

%clean
rm -rf %buildroot

%files
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog README INSTALL NEWS
%{_bindir}/%{name}
%{_datadir}/%{name}/pixmaps/*.png
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%{_datadir}/icons/hicolor/*/apps/*.*
%{_datadir}/applications/imagination.desktop
%{_libdir}/%{name}/*.so

%changelog
* Sun Mar 29 2009 Jack Fung - 1.0-1
- initial spec.
