# Activity Monitor

Activity Monitor - это клиент-серверное приложение для мониторинга рабочей активности сотрудников в организации.

## Функциональные возможности

- Отображение текущей рабочей активности всех сотрудников
- Отправка данных о системе клиента (имя пользователя, имя компьютера, домен)
- Захват и передача скриншотов с рабочего стола клиента
- Веб-интерфейс для просмотра активности и скриншотов

## Клиентская часть

Клиентская часть написана на C/C++ и выполняет следующие задачи:

1. Сбор информации о системе (имя пользователя, имя компьютера, домен)
2. Захват скриншота рабочего стола
3. Отправка данных и скриншота на сервер
4. Автоматический запуск при входе пользователя в систему

## Серверная часть (Python + Flask)

Серверная часть написана на Python с использованием Flask и выполняет следующие задачи:

1. Прием данных от клиентов
2. Сохранение данных в базу данных SQLite
3. Отображение данных в веб-интерфейсе

### Компиляция и запуск клиента

Для компиляции клиента используйте любой компилятор C/C++ (например, g++):

```sh
g++ -o ActivityMonitor ActivityMonitor.cpp -lgdiplus -lwininet
```

# Установка клиента в автозапуск
Код автоматически добавляет клиент в автозапуск при первом запуске, используя реестр Windows.

# Серверная часть
Серверная часть написана на Python с использованием Flask и выполняет следующие задачи:

1. Прием данных от клиентов
2. Сохранение данных в базу данных SQLite
3. Отображение данных в веб-интерфейсе

# Установка и запуск сервера
## Установите зависимости:
```sh
pip install flask
```
## Инициализируйте базу данных и запустите сервер:
```sh
python server.py
```
# Веб-интерфейс
Веб-интерфейс доступен по умолчанию по адресу ```http://127.0.0.1:5000```. В нем отображается список всех подключенных клиентов и их последняя активность, а также скриншоты рабочего стола.# Client-Server-appl-for-monitoring


### Дополнительно

- Убедитесь, что путь к клиенту в `addToStartup()` правильный.
- Внесите любые необходимые изменения, чтобы настроить приложение под ваши нужды.