import sqlite3

def init_db():
    with sqlite3.connect('activity.db') as conn:
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS activity
                     (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, computername TEXT, domainname TEXT, last_active TEXT)''')
        c.execute('''CREATE TABLE IF NOT EXISTS screenshots
                     (id INTEGER PRIMARY KEY AUTOINCREMENT, activity_id INTEGER, timestamp TEXT, image BLOB, FOREIGN KEY(activity_id) REFERENCES activity(id))''')
        conn.commit()
