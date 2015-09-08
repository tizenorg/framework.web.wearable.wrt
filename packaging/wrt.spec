#git:framework/web/wrt
Name:       wrt
Summary:    web runtime
Version:    0.8.325_w21
Release:    1
Group:      Development/Libraries
License:    Apache-2.0 and Flora-1.1
URL:        N/A
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext
BuildRequires:  libcap-devel
BuildRequires:  libss-client-devel
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-appfw-watch-application)
BuildRequires:  pkgconfig(appcore-watch)
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(cert-svc)
BuildRequires:  pkgconfig(cert-svc-vcore)
BuildRequires:  pkgconfig(deviced)
BuildRequires:  pkgconfig(dpl-efl)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(efl-assist)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(ewebkit2)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(json-glib-1.0)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(libiri)
BuildRequires:  pkgconfig(libpcrecpp)
BuildRequires:  pkgconfig(libprivilege-control)
BuildRequires:  pkgconfig(libsmack)
BuildRequires:  pkgconfig(libsoup-2.4)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(secure-storage)
BuildRequires:  pkgconfig(security-client)
BuildRequires:  pkgconfig(security-core)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(wrt-plugin-loading)
BuildRequires:  pkgconfig(wrt-plugin-js-overlay)
BuildRequires:  pkgconfig(wrt-plugins-ipc-message)
BuildRequires:  pkgconfig(wrt-popup-wrt-runner)
BuildRequires:  pkgconfig(wrt-popup-ace-runner)
BuildRequires:  pkgconfig(journal)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(privacy-manager-client)
BuildRequires:  libss-client-devel
BuildRequires:  gettext
BuildRequires:  edje-tools
Requires: libss-client

### NETWORK TRACE #############################################################
BuildRequires:  pkgconfig(libresourced)
###############################################################################

## wrt-launchpad-daemon #######################################################
BuildRequires:  pkgconfig(app-checker)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(libsmack)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(libsystemd-daemon)
%{?systemd_requires}

#Use these macro to avoid hard-coded path
#After upgrading systemd to v204 or higher macro can be deleted
%define _unitdir /usr/lib/systemd/system
###############################################################################

%description
web runtime

%package devel
Summary:  Wrt header files for external modules
Group:    Development/Libraries
Requires: %{name} = %{version}
Requires: pkgconfig(ewebkit2)

%description devel
wrt library development headers

%prep
%setup -q

%define with_tests 0
%if "%{WITH_TESTS}" == "ON" || "%{WITH_TESTS}" == "Y" || "%{WITH_TESTS}" == "YES" || "%{WITH_TESTS}" == "TRUE" || "%{WITH_TESTS}" == "1"
    %define with_tests 1
%endif

%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"

export CFLAGS="$CFLAGS -DTIZEN_RELEASE_TYPE_ENG"
export CXXFLAGS="$CXXFLAGS -DTIZEN_RELEASE_TYPE_ENG"
export FFLAGS="$FFLAGS -DTIZEN_RELEASE_TYPE_ENG"

export LDFLAGS+="-Wl,--rpath=/usr/lib"

%ifarch %{arm}
%define build_dir Build-arm
%else
%define build_dir Build-i586
%endif

mkdir -p %{build_dir}
cd %{build_dir}

cmake .. -DCMAKE_INSTALL_PREFIX=%{_prefix} \
        -DDPL_LOG="ON"                    \
        -DPROJECT_VERSION=%{version} \
        -DCMAKE_BUILD_TYPE=%{?build_type:%build_type} \
        %{?WITH_TESTS:-DWITH_TESTS=%WITH_TESTS}
make %{?jobs:-j%jobs}

%install
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
cp LICENSE.Flora %{buildroot}/usr/share/license/%{name}

cd %{build_dir}
%make_install
mkdir -p %{buildroot}%{app_data}

mkdir -p %{buildroot}%{_unitdir}/multi-user.target.wants
ln -s %{_unitdir}/wrt_launchpad_daemon.service %{buildroot}%{_unitdir}/multi-user.target.wants/wrt_launchpad_daemon.service

%pre
if [ $1 -eq 2 ] ; then
    systemctl stop wrt_launchpad_daemon.service
fi

%preun
if [ $1 -eq 0 ] ; then
    systemctl stop wrt_launchpad_daemon.service
fi

%clean
rm -rf %{buildroot}

%post
chmod +s /usr/bin/wrt-launcher

mkdir -p %{_sysconfdir}/systemd/default-extra-dependencies/ignore-units.d/
ln -s %{_libdir}/systemd/system/wrt_launchpad_daemon.service %{_sysconfdir}/systemd/default-extra-dependencies/ignore-units.d/

/sbin/ldconfig
systemctl daemon-reload
if [ $1 -eq 2 ] ; then
    systemctl start wrt_launchpad_daemon.service
fi

%postun
systemctl daemon-reload

%files
%manifest wrt.manifest
%{_libdir}/*.so
%{_libdir}/*.so.*
%attr(755,root,root) %{_bindir}/wrt-client
%attr(755,root,root) %{_bindir}/wrt-launcher
%attr(755,root,root) %{_bindir}/wrt_reset_all.sh
%attr(755,root,root) %{_bindir}/wrt_reset_db.sh
%{_datadir}/locale/*
%{_datadir}/license/%{name}
%attr(644,root,root) %{_datadir}/edje/wrt/*
%attr(644,root,root) %{_datadir}/edje/ace/*
%if %{with_tests}
    %attr(755,root,root) %{_bindir}/wrt-tests-general
    /opt/share/widget/tests/general/*
%endif
%attr(755,root,root) %{_sysconfdir}/profile.d/wrt_env.sh

## wrt-launchpad-daemon #######################################################
%attr(755,root,root) %{_bindir}/wrt_launchpad_daemon
/usr/share/aul/preload_list_wrt.txt
/etc/smack/accesses.d/wrt_launchpad_daemon.efl
/etc/smack/accesses.d/wrt-launcher.efl
#systemd
%{_unitdir}/multi-user.target.wants/wrt_launchpad_daemon.service
%{_unitdir}/wrt_launchpad_daemon.service
###############################################################################

%files devel
%{_includedir}/*
%{_libdir}/pkgconfig/*
