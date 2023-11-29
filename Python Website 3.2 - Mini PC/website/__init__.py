#SET UP FOR FLASK APPLICATION

from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from os import path
from flask_login import LoginManager

db = SQLAlchemy()
DB_NAME = "database.db"


def create_app():
    # __name__ is just the file name, this is basic FLASK set up, don't need to know what this means, just has to be here.
    app = Flask(__name__)
    # For all of the Flask applications we have a SECRET_KEY config variable.
    # This is going to encrypt/secure the cookies and session data related to our website.
    # This is just a random string, you can make it whatever you want.
    app.config['SECRET_KEY'] = 'hjshjhdjah kjshkjdhjs'
    #SQLite/SQLAlchemy database is stored at this location. This will store the database in the website folder.
    app.config['SQLALCHEMY_DATABASE_URI'] = f'sqlite:///{DB_NAME}'
    #Using database with flash app
    db.init_app(app)

    # import blueprints from views and auth files
    from .views import views
    from .auth import auth

    # Register blueprints
    # The URL prefix is saying "all of the URLs stored inside of this blueprints file, how do I access them?
    app.register_blueprint(views, url_prefix='/')
    app.register_blueprint(auth, url_prefix='/')
    
    #.model for relative import
    # have to update when adding more database objects
    from .models import User, Note, Patient, Session #NEW CODE
    
    with app.app_context():
        db.create_all()

    login_manager = LoginManager()
    login_manager.login_view = 'auth.login'
    login_manager.init_app(app)

    @login_manager.user_loader
    def load_user(id):
        return User.query.get(int(id))

    return app

# checks if the database exists so it doesn't write over already existing databases
def create_database(app):
    if not path.exists('website/' + DB_NAME):
        # Have to pass it app so the database knows which app we are creating it for (app defined line 20)
        db.create_all(app=app)
        print('Created Database!')
