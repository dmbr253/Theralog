# DATABASE MODELS

# from . is the same as from website
from . import db
from .forms import PatientForm #NEW CODE
from flask_login import UserMixin
from sqlalchemy.sql import func


class Note(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    data = db.Column(db.String(10000))
    # New notes will automatically add the date for us
    date = db.Column(db.DateTime(timezone=True), default=func.now())
    # Foreign Key is a key in a database table that always references an ID to another database column
    # in SQL foreign key name User will be lower case user
    # One to many relationship
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))

# In python class names are upper case
class User(db.Model, UserMixin):
    # what if users had the same first name? You need a unique id (primary_key) for each user
    id = db.Column(db.Integer, primary_key=True)
    email = db.Column(db.String(150), unique=True)
    password = db.Column(db.String(150))
    first_name = db.Column(db.String(150))
    folder_path = db.Column(db.String(255))
    # list storing all previouse notes
    # In SQL relationship name is upper case, Notes
    notes = db.relationship('Note')
    patients = db.relationship('Patient')  # Define the relationship to Patient model

class Patient(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    behaviors = db.Column(db.String(1000), nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))  # Define the foreign key

class Session(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))  # Assuming a relationship with a User model
    patient_id = db.Column(db.Integer, nullable=False)
    session_id = db.Column(db.Integer, nullable=False)
    additional_information = db.Column(db.String(1000))
