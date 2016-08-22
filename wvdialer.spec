%global cmake_build_dir build-cmake

Name:          WvDialer
Version:       1.0
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

BuildRequires: desktop-file-utils
BuildRequires: qt5-qtbase-devel
BuildRequires: kf5-kauth-devel
BuildRequires: kf5-knotifications-devel
BuildRequires: extra-cmake-modules

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

%files
%license COPYING
%doc README.md Changelog
%{_bindir}/%{name}
%{_libexecdir}/kf5/kauth/wvdialer_helper
%{_datadir}/applications/%{name}.desktop
%{_datadir}/dbus-1/system-services/pro.russianfedora.wvdialer.service
%{_datadir}/polkit-1/actions/pro.russianfedora.wvdialer.policy
%{_sysconfdir}/dbus-1/system.d/pro.russianfedora.wvdialer.conf
%{_datadir}/knotifications5/%{name}.notifyrc

%changelog
* Sun Aug 21 2016 Fl@sh <kaperang07@gmail.com> - 1.0-1
- Initial build
