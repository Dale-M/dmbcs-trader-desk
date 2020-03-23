> Github is a secondary distribution point for this project; please see
> https://rdmp.org/dmbcs/trader-desk to be sure to see the most up to date
> version and complete details.

# DMBCS Trader-Desk Application

> This is a beta release of C++ code designed to be built and run on a
> Gnu/Linux-based system.

While stock charting applications are ten a penny, it seems like every
time you look at a different stock chart you see a completely different
picture of the situation.  Especially when it is your broker: things
always look great until the time you buy in and then you see things are
all really downhill.

So we want a desk which shows us data we can believe, and allows us to
very quickly view and analyze the data in lots of ways, at lots of time
scales; we want to be live and interactive with our data.

Specifically, our requirements are

* Show full, at-a-glance, view of an entire market
* Instantly bring up all the details of any one company
* Have all data (ten years, all markets) on local machine, in a
    proper database, for speed
* Application must be lightning fast, we want to pan and
  zoom around the data to our heart’s content
  * Ultimately we want trading robots which can react to
    events super-fast
* Provide plug-in infrastructure for analysis modules, and
    ultimately for robot traders

## The Project

We are a team of professional C++ Linux programmers and this has been a
personal side project, and thus isn’t really designed for portability or
ease of installation.  But we have made an effort, and the application
does include a wizard which will go some way towards getting your database
set up correctly.

### Prerequisites

The requirements are specific and quite hard:

* `gcc 9.3`
    * Yep, this is written to C++20 standards with **concepts** and the
      **fmt** library in use.
* `fmt`
* `mariadb`
* `dmbcs-market-data-api`
* An account at *AlphaVantage* (free); you will need to acquire an *API
  key* from that site and enter it at the bottom of the preferences dialog
  box in the `trader-desk` application

It has only been built on a Debian 10 (current stable) system.  The file
`setup-hint.sh` at the root of the distribution is a pseudo-script for
configuring and installing into a clean-built Debian 10 machine.  It is
not expected that you would just run this blindly, but use it as a guide
for manually setting your own system up at the command line; in particular
it will inform you how to get the dependencies listed above.


## Download

The `dmbcs-trader-desk` source code is managed with *GIT* (configured with
*autotools*, built with *make* and a good C++20 compiler).  Type

> `git clone http://rdmp.org/dmbcs/trader-desk.git dmbcs-trader-desk`

at the command line to obtain a copy.

This repository also comes with some database pre-population data, so that
you will have something interesting to look at as soon as you start the
application running!

## Documentation

As per above, build and installation instructions take the form of the
`setup-hint.sh` pseudo-script included with the sources.  The
source is about 50% covered by *Doxygen* notes in the header files.

Real end-user documentation is non-existent right now.  You should have
had a look at the video above, and then you will be able to follow your
instincts and find your own way around the application.

## Contact
      
Please click [here](https://rdmp.org/dmbcs/contact) if you wish to send us
a message.

### Mailing list

If you would like to receive e-mail notices of matters arising about this
application, you may request this through the contact form above.

### Contribution to development

We will happily consider contributions to the source code if you provide
the address of a GIT repository we can pull from, or send a pull request
via Github, and will consider all bug reports and feature requests.

## Donations

If you use this application please consider a bitcoin donation if you can.
A small amount informs us that there is interest and that we are providing
a useful service to the community; it will keep us motivated to continue
to make open source software.  Donations can be made by bitcoin to the
address `1PWHez4zT2xt6PoyuAwKPJsgRznAKwTtF9`.


______________________________________________________________________
Copyright (c) 2017, 2020  Dale Mellor

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.
