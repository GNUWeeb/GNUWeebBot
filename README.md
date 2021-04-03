# GNUWeebBot
GNUWeeb Telegram Bot


# Features
- [x] Event Debug
- [ ] Event Logger
- [ ] Group Management


# Requirements
- gcc 9.2.1+ or clang 11
- GNU Make 4.3
- OpenSSL 1.1.1
- libmysqlclient21
- libcurl4-openssl-dev


# Install Dependency
### Ubuntu
```sh
sudo apt install -y git gcc g++ make cmake libmysqlclient21 libmysqlclient-dev libcurl4-openssl-dev valgrind doxygen;
```

### Arch
```sh
sudo pacman -Syu git base-devel cmake curl libmysqlclient21 valgrind doxygen;
```


# Build
```sh
git clone https://github.com/GNUWeeb/GNUWeebBot;
cd GNUWeebBot;

make RELEASE_MODE=1 -j$(nproc);

cp config/example.ini config.ini;

# Fix the config
vim config.ini;

./gwbot -c config.ini;
```

# Issues
We welcome bug reports, feature requests and questions through GitHub
repository https://github.com/GNUWeeb/GNUWeebBot


# Project Maintainers
- Ammar Faizi ([@ammarfaizi2](https://github.com/ammarfaizi2))
- Aliwoto ([@ALiwoto](https://github.com/ALiwoto))


# Community
We are online on Telegram, see: https://t.me/GNUWeeb


# Third Party Libraries

| No.   | Link                                    | Name                  | License                   |
|:-----:|:----------------------------------------|:----------------------|:--------------------------|
| 1.    | https://github.com/benhoyt/inih         | inih 53               | New BSD license           |
| 2.    | https://github.com/openssl/openssl      | OpenSSL 1.1.1         | Apache-2.0                |
| 3.    | https://github.com/axboe/liburing       | liburing-2.0          | LGPL + MIT                |
| 4.    | https://github.com/curl/curl            | libcurl4-openssl-dev  | MIT                       |
| 5.    | https://github.com/json-c/json-c        | json-c-0.15-20200726  | MIT                       |
| 6.    | https://dev.mysql.com/downloads/c-api   | MySQL 8               | GPL-v2                    |


# Contributing
We welcome contributors through GitHub pull request. Please read the
`CONTRIBUTING.md` file for detailed information.


# License
This software is licensed under the GNU GPL-v2 license.
