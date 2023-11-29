---------------------------------------WEB SERVER README------------------------------------------
You MUST create a folder called "Local_Sessions_File" on your device's desktop and alter 
the following lines in views.py to point to that folder's directory.

Line 148 in the views.py = "source_dir = 'C:\\Users\\GraceAnn\\Desktop\\Local_Session_Files'" 
Line 174 in the views.py = "source_dir = 'C:\\Users\\GraceAnn\\Desktop\\Local_Session_Files'"

---------------------------------------------------------------------------
Git hub code used to start web server: YouTube video is very helpful.
github.com/techwithtim/Flask-Web-App-Tutorial
https://www.youtube.com/watch?v=dam0GPOAvVI&t=2276s&ab_channel=TechWithTim 
---------------------------------------------------------------------------
## Setup & Installation

Make sure you have the latest version of Python installed.

```bash
git clone <repo-url>
```

```bash
pip install -r requirements.txt
```

## Running The App

```bash
python main.py
```

## Viewing The App

Local Network: (only device running web server can connect to this link)
Go to `http://127.0.0.1:5000`

Not Local Network:
After running web server by typing "python main.py" it will display the URL the webserver is running on.

---------------------------------------
Current pip installations on my device:
Package           Version
----------------- -----------
alembic           1.12.0
blinker           1.6.2
click             8.1.7
colorama          0.4.6
dnspython         2.4.2
EasyProcess       1.1
email-validator   2.1.0.post1
entrypoint2       1.1
Flask             2.3.3
Flask-Login       0.6.2
Flask-Mail        0.9.1
Flask-Migrate     4.0.5
Flask-Script      2.0.6
Flask-SQLAlchemy  3.0.5
Flask-WTF         1.1.1
greenlet          2.0.2
idna              3.4
itsdangerous      2.1.2
Jinja2            3.1.2
Mako              1.2.4
MarkupSafe        2.1.3
MouseInfo         0.1.3
mss               9.0.1
Pillow            10.1.0
pip               23.2.1
PyAutoGUI         0.9.54
PyGetWindow       0.0.9
PyMsgBox          1.0.9
pyperclip         1.8.2
PyRect            0.2.0
pyscreenshot      3.1
PyScreeze         0.1.29
pytweening        1.0.7
setuptools        65.5.0
SQLAlchemy        2.0.20
typing_extensions 4.7.1
Werkzeug          2.3.7
WTForms           3.0.1
