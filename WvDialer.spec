%global cmake_build_dir build-cmake

Name:          WvDialer
Version:       1.1
Release:       1%{?dist}
Summary:       Dialer wrapped over wvdial
Summary(ru):   Программа дозвона для модемов мобильных устройств
License:       GPLv2+
Source0:       https://github.com/F1ash/%{name}/archive/%{version}.tar.gz
URL:           https://github.com/F1ash/%{name}

Requires:      qt5-qtbase
Requires:      kf5-kauth
Requires:      kf5-knotifications
Requires:      dbus
Requires:      systemd
Requires:      polkit
Requires:      wvdial
Requires:      hicolor-icon-theme

BuildRequires: desktop-file-utils
BuildRequires: qt5-qtbase-devel
BuildRequires: kf5-kauth-devel
BuildRequires: kf5-knotifications-devel
BuildRequires: extra-cmake-modules
%{?systemd_requires}
BuildRequires: systemd

%description
WvDialer
Dialer wrapped over wvdial for mobile modems.
Implemented auto detect device, connect.

%description -l ru
WvDialer
Программа дозвона для модемов мобильных устройств.
Автообнаружение устройства и соединение.

%prep
%setup -q

%build
mkdir %{cmake_build_dir}
pushd %{cmake_build_dir}
      %cmake ..
      %{make_build}
popd

%install
pushd %{cmake_build_dir}
      %{make_install}
popd

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/%{name}.desktop

%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
%systemd_post %{name}@.service

%preun
%systemd_preun %{name}@.service

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi
%systemd_postun %{name}@.service

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :

%files
%doc README.md
%{_bindir}/%{name}
%{_libexecdir}/kf5/kauth/wvdialer_helper
%{_datadir}/applications/%{name}.desktop
%{_datadir}/dbus-1/system-services/pro.russianfedora.wvdialer.service
%{_datadir}/polkit-1/actions/pro.russianfedora.wvdialer.policy
%{_sysconfdir}/dbus-1/system.d/pro.russianfedora.wvdialer.conf
%{_datadir}/knotifications5/%{name}.notifyrc
%{_unitdir}/%{name}.service
%{_datadir}/icons/hicolor/64x64/apps/%{name}.png

%changelog
* Thu Feb  2 2017 Fl@sh <kaperang07@gmail.com> - 1.1-1
- version updated;
- added %%post, %%preun, %%postun, %%posttrans macroses;

* Thu Sep  1 2016 Fl@sh <kaperang07@gmail.com> - 1.0-1
- Initial build
