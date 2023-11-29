#DEFINE LOG IN, LOG OUT, SIGN UP

from flask import Blueprint, render_template, request, flash, redirect, url_for
#import user from models so it can be used in this file
from .models import User
# Password hash, ensures that all passwords are not saved in plain text, security, Only saves hashed password to compare with user input
from werkzeug.security import generate_password_hash, check_password_hash
from . import db   #means from __init__.py import db
from flask_login import login_user, login_required, logout_user, current_user


auth = Blueprint('auth', __name__)

# methods are the types of requests accepted by this route. GET request = URL, POST request = submit
@auth.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        email = request.form.get('email')
        password = request.form.get('password')

        # looking for specific email, .first returns first result
        user = User.query.filter_by(email=email).first()
        # If we found a user, check password
        if user:
            if check_password_hash(user.password, password):
                flash('Logged in successfully!', category='success')
                login_user(user, remember =True)
                return redirect(url_for('views.display_patients'))
            else:
                flash('Incorrect password, try again.', category='error')
        else:
            flash('Email does not exist.', category='error')

    return render_template("login.html", user=current_user)


@auth.route('/logout')
@login_required
def logout():
    logout_user()
    return redirect(url_for('auth.login'))


from os.path import expanduser
import os

@auth.route('/sign-up', methods=['GET', 'POST'])
def sign_up():
    if request.method == 'POST':
        email = request.form.get('email')
        first_name = request.form.get('firstName')
        password1 = request.form.get('password1')
        password2 = request.form.get('password2')

        # Password, Email checks
        user = User.query.filter_by(email=email).first()
        if user:
            # Flash a message
            flash('Email already exists.', category='error')
        elif len(email) < 4:
            flash('Email must be greater than 3 characters.', category='error')
        elif len(first_name) < 2:
            flash('First name must be greater than 1 character.', category='error')
        elif password1 != password2:
            flash('Passwords don\'t match.', category='error')
        elif len(password1) < 7:
            flash('Password must be at least 7 characters.', category='error')
        else:
            # Create a directory for the user on the desktop
            user_folder_name = f"{first_name}_{email}"
            user_folder_path = os.path.join(expanduser("~"), "Desktop", user_folder_name)
            os.makedirs(user_folder_path, exist_ok=True)

            # sha256 is a hashing algorithm
            new_user = User(email=email, first_name=first_name, password=generate_password_hash(password1, method='sha256'), folder_path=user_folder_path)
            # Add new user to the database
            db.session.add(new_user)
            # Update changes in the database
            db.session.commit()
            login_user(new_user, remember=True)
            flash('Account created!', category='success')
            # return user to the home page after user sign up
            return redirect(url_for('views.display_patients'))

    return render_template("sign_up.html", user=current_user)

