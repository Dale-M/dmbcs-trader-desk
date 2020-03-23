#!/bin/bash

# Get an up to date system with all the packages we require.

sudo apt update
sudo apt upgrade -y
sudo apt install -y  git build-essential libmariadbclient-dev \
                     mariadb-server autoconf autopoint        \
                     libssl-dev libtool libgtkmm-3.0-dev      \
                     libcurl4-openssl-dev cmake texinfo xauth \
                     gettext ed


# Set up the database to make it possible for anybody (in particular the
# trader-desk application) to use the database root account.

sudo mysql mysql  <<-\EOF
		update mysql.user set password='', plugin='';
		flush privileges;
		exit
	EOF


# Download, build and install gcc 9.3.  We use the latest C++20 standards,
# and need the best compiler we can get.

sudo bash -c 'cat > /etc/ld.so.conf.d/01-local.conf' <<-\EOF  
		/usr/local/lib64
		/usr/local/lib
	EOF
sudo ldconfig

mkdir ${HOME}/sources;  cd ${HOME}/sources
wget  ftp://ftp.gnu.org/gnu/gcc/gcc-9.3.0/gcc-9.3.0.tar.xz
wget  ftp://ftp.gnu.org/gnu/gmp/gmp-6.2.0.tar.xz
wget  ftp://ftp.gnu.org/gnu/mpc/mpc-1.1.0.tar.gz
wget  ftp://ftp.gnu.org/gnu/mpfr/mpfr-4.0.2.tar.xz
tar xf gcc-9.3.0.tar.xz
cd gcc-9.3.0
tar xf ../gmp-6.2.0.tar.xz
ln -s gmp-6.2.0 gmp
tar xf ../mpc-1.1.0.tar.gz
ln -s mpc-1.1.0 mpc
tar xf ../mpfr-4.0.2.tar.xz
ln -s mpfr-4.0.2 mpfr
./configure --enable-languages=c,c++ --disable-bootstrap --disable-multilib
make -j2    #  Takes a long time.
sudo make install
cd ${HOME}/sources;  rm -rf gcc-9.3.0


# Make a couple of third-party packages that don’t come with the Debian
# system.

cmake_build()  {  cd $1
                  mkdir build
                  cd build
                  cmake -DBUILD_SHARED_LIBS=TRUE ..
                  make -j2
                  sudo make install  
}


cd ${HOME}/sources
git clone https://github.com/fmtlib/fmt.git
( cmake_build fmt )


cd ${HOME}/sources
git clone https://github.com/jpbarrette/curlpp.git
( cmake_build curlpp )

sudo ed /usr/local/lib/pkgconfig/curlpp.pc <<-\EOF
		1,$s@-Llib@-L${prefix}/lib@
		w
		q
	EOF
		

# Now build the DMBCS packages which make up the trader-desk application.

autotools_build()  {  cd $1
                      autoreconf --install
                      ./configure
                      make -j2
                      sudo make install    
}

cd ${HOME}

git clone https://rdmp.org/dmbcs/market-data-api.git dmbcs-market-data-api
( autotools_build dmbcs-market-data-api )

git clone https://rdmp.org/dmbcs/trader-desk.git dmbcs-trader-desk
( autotools_build dmbcs-trader-desk )

sudo ldconfig

mkdir ${HOME}/.config


# Take heed of this message!

cat <<EOF

    The dmbcs-trader-desk installation is complete.  If you have just done
    this on a fresh Debian installation, you will now need to log out and
    then back in again with the -X option on the ssh line, so that
    graphics will appear in front of you.

    Then just type  trader-desk.

EOF
