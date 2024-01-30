Name:    xrootd-ceph-ckslib
Version: {{ckslib_version}}
Release: {{ckslib_release}}%{?dist}
Summary: Checksum library for xrootd server with ceph backend	

Group: XrootD plugins
License: LGPLv3+
URL: https://github.com/alex-rg/xrd_ckslib
Source0: xrdckslib.tar.gz

BuildRequires: gcc
BuildRequires: make

%define xrootd_version v{{xrootd_version}}
%define xrootd_branch {{xrootd_branch}}
%define xrootd_repo {{xrootd_repo}}

%description
Custom checksum library for xrootd with ceph backend.

%prep
%setup -c
mkdir xrtd_src
cd xrtd_src
git clone -b %{xrootd_branch} %{xrootd_repo} .
git reset --hard %{xrootd_version}
./genversion.sh
cd ..


%build
make libXrdCksPlugin.so INCLUDE='-I./xrtd_src/src' %{?_smp_mflags}


%install
install -m 0755 -d %{buildroot}%{_libdir}
install -m 0644 libXrdCksPlugin.so %{buildroot}%{_libdir}/libXrdCksPlugin.so


%files
%{_libdir}/libXrdCksPlugin.so


%changelog
* Wed Jan 03 2024 alexrg - 0.0.1
 - First version
