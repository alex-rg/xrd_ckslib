Name:    xrootd-ceph-ckslib
Version: 0.0
Release: 1%{?dist}
Summary: Checksum library for xrootd server with ceph backend	

Group: XrootD plugins
License: LGPLv3+
URL: https://github.com/alex-rg/xrd_ckslib
Source0: xrdckslib.tar.gz
Source1: xrootd.tar.gz

BuildRequires: gcc
BuildRequires: make

%define xrootd_version v5.5.1
%define xrootd_branch v5.5.x
%define xrootd_repo https://github.com/xrootd/xrootd.git

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
