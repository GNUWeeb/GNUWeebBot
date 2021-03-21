# GNUWeebBot
GNUWeeb Telegram Bot


# Requirements
- gcc 9.2.1+
- GNU Make 4.3
- OpenSSL 1.1.1
- libcurl4-openssl-dev


# Build
```
sudo apt install gcc g++ make libcurl4-openssl-dev valgrind
git clone https://github.com/GNUWeeb/GNUWeebBot
cd GNUWeebBot
make RELEASE_MODE=1 -j$(nproc)
cp config/example.ini config.ini
vim config.ini # Fix the config
./gwbot config.ini
```

# Issues
We welcome bug reports, feature requests and questions through GitHub
repository https://github.com/GNUWeeb/GNUWeebBot


# Project Maintainers
- Ammar Faizi ([@ammarfaizi2](https://github.com/ammarfaizi2))
- ALi.w ([@ALiwoto](https://github.com/ALiwoto))


# Community
We are usually online on Telegram, see: https://t.me/GNUWeeb


# Third Party Libraries
- inih 53 (under New BSD license) (https://github.com/benhoyt/inih)
- Criterion v2.3.3 (under MIT license) (https://github.com/Snaipe/Criterion)
- OpenSSL 1.1.1 (under Apache-2.0 license) (https://github.com/openssl/openssl)
- liburing-2.0 (under LGPG + MIT) (https://github.com/axboe/liburing)
- libcurl4-openssl-dev 7.74.0 (MIT) (https://github.com/curl/curl)


# Contributing
We welcome contributors through GitHub pull request. Please read the
`CONTRIBUTING.md` file for detailed information.


# License
This software is licensed under the GNU GPL-v2 license.
