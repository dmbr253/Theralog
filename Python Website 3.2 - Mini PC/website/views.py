#STORE STANDARD ROOTS FOR WEBSITE WHERE USERS CAN GO TO (homepage)

# We are going to define that this file is a blueprint of our application. Meaning, it has a bunch of roots inside of it (URLs defined in here)
from flask import Blueprint, render_template, request, flash, jsonify, redirect, url_for
from flask_login import login_required, current_user
from .models import Note
from .models import Patient
from .models import Session
from .forms import PatientForm
from .forms import EditPatientForm
from .forms import EditSessionForm
from .forms import SessionForm
from .models import User

from . import db
import json

# Naming blueprint 'views', ingore __name__ (flask thing)
views = Blueprint('views', __name__)

#This home function will run when we go to our root (/) meaning the URl
# This is called a decorator of a function
#@views.route(methods=['GET', 'POST'])

def home():
    if request.method == 'POST': 
        note = request.form.get('note')#Gets the note from the HTML 

        if len(note) < 1:
            flash('Note is too short!', category='error') 
        else:
            new_note = Note(data=note, user_id=current_user.id)  #providing the schema for the note 
            db.session.add(new_note) #adding the note to the database 
            db.session.commit()
            flash('Note added!', category='success')

    return render_template("home.html", user=current_user)


@views.route('/delete-note', methods=['POST'])
def delete_note():  
    note = json.loads(request.data) # this function expects a JSON from the INDEX.js file 
    noteId = note['noteId']
    note = Note.query.get(noteId)
    if note:
        if note.user_id == current_user.id:
            db.session.delete(note)
            db.session.commit()

    return jsonify({})


from flask import render_template, request, flash, redirect, url_for
from flask_login import current_user  # Import current_user from your authentication system
from sqlalchemy.exc import IntegrityError
from .models import db, Patient  # Import your SQLAlchemy models
from .forms import PatientForm  # Import your form class

# Your route definition
@views.route('/add_patient', methods=['GET', 'POST'])
@login_required
def add_patient():
    form = PatientForm()  # Create the form object

    if request.method == 'POST':
        if form.validate():
            # Attempt to add the patient to the database
            try:
                new_patient = Patient(id=form.id.data, behaviors=form.behaviors.data, user_id=current_user.id)  # Assuming user_id is set from current_user
                db.session.add(new_patient)
                db.session.commit()
                flash('Patient added successfully', 'success')
                return redirect(url_for('views.display_patients'))
            except IntegrityError as e:
                db.session.rollback()
                flash('This patient ID already exists. Please use a different ID.', 'error')
        else:
            flash('Form validation failed. Please check your inputs.', 'danger')

    return render_template('add_patient.html', form=form, user=current_user)



@views.route('/', methods=['GET', 'POST'])
@login_required
def display_patients():
    user = current_user
    search_query = request.form.get('search_query', '')

    if search_query:
        patients = Patient.query.filter_by(user_id=user.id).filter(
            (Patient.id.ilike(f"%{search_query}%")) | 
            (Patient.behaviors.ilike(f"%{search_query}%"))
        ).all()
    else:
        patients = Patient.query.filter_by(user_id=user.id).all()

    return render_template('patients.html', patients=patients, user=user, search_query=search_query)


@views.route('/delete-patient/<int:id>', methods=['POST'])
@login_required
def delete_patient(id):
    patient = Patient.query.get(id)
    if patient:
        db.session.delete(patient)
        db.session.commit()
        flash('Patient deleted successfully!', category='success')
    return redirect(url_for('views.display_patients'))

@views.route('/edit-patient/<int:id>', methods=['GET', 'POST'])
@login_required
def edit_patient(id):
    patient = Patient.query.get(id)
    form = EditPatientForm(obj=patient)

    if form.validate_on_submit():
        # Update patient details with data from the form
        form.populate_obj(patient)
        db.session.commit()
        flash('Patient details updated successfully!', category='success')
        return redirect(url_for('views.display_patients'))

    return render_template('edit_patient.html', form=form, patient=patient, user=current_user)

from .forms import SessionForm
from .models import Session


# Import the necessary modules
import os
# Import the necessary modules
import os
import shutil
from flask import Blueprint, render_template, request, flash, redirect, url_for
# Other imports...

import os
import shutil

@views.route('/add_session', methods=['GET', 'POST'])
@login_required
def add_session():
    form = SessionForm()
    form.patient_id.choices = [(str(patient.id), patient.id) for patient in Patient.query.filter_by(user_id=current_user.id)]

    if form.validate_on_submit():
        source_dir = 'C:\\Users\\GraceAnn\\Desktop\\Local_Session_Files'
        # Check if the "Local_Session_Files" directory is empty
        if not any(os.scandir(source_dir)):
            flash('You cannot create a session until the session files are available', 'error')
            return redirect(url_for('views.add_session'))  # Redirect them back to the add_session page

        # Since there are files, create a new session with the correct user and patient IDs
        patient_id = int(form.patient_id.data)
        session_id = form.session_id.data
        additional_information = form.additional_information.data

        new_session = Session(user_id=current_user.id, patient_id=patient_id, session_id=session_id, additional_information=additional_information)
        db.session.add(new_session)
        db.session.commit()

        # Move and rename the files from "Local_Session_Files" to the user's folder
        move_and_rename_session_files(current_user.folder_path, patient_id, session_id)

        flash('Session added successfully', 'success')
        return redirect(url_for('views.display_sessions'))

    return render_template('add_session.html', form=form, user=current_user)


def move_and_rename_session_files(destination, patient_id, session_id):
    # Source directory
    source_dir = 'C:\\Users\\GraceAnn\\Desktop\\Local_Session_Files'
    
    # Ensure the destination directory exists
    os.makedirs(destination, exist_ok=True)

    # Check if the "Local_Session_Files" directory is empty
    if not any(os.scandir(source_dir)):
        flash('You cannot create a session until a session folder is available', 'error')
        return

    # Move all files from source_dir to destination and rename them
    for filename in os.listdir(source_dir):
        file_path = os.path.join(source_dir, filename)
        if os.path.isfile(file_path):
            # Construct new filename
            new_filename = f"PatientID_{patient_id}_SessionID_{session_id}_{filename}"
            new_file_path = os.path.join(destination, new_filename)
            # Move and rename the file
            shutil.move(file_path, new_file_path)


@views.route('/sessions', methods=['GET', 'POST'])
@login_required
def display_sessions():
    user = current_user
    search_query = request.form.get('search_query', '')

    if request.method == 'POST':
        search_query = request.form.get('search_query', '')

    if search_query:
        sessions = Session.query.filter_by(user_id=user.id).filter(
            (Session.session_id.ilike(f"%{search_query}%")) |
            (Session.additional_information.ilike(f"%{search_query}%"))
        ).all()
    else:
        sessions = Session.query.filter_by(user_id=user.id).all()

    return render_template('sessions.html', sessions=sessions, user=user, search_query=search_query)
 

@views.route('/edit-session/<int:id>', methods=['GET', 'POST'])
@login_required
def edit_session(id):
    session = Session.query.get(id)
    form = EditSessionForm(obj=session)

    if form.validate_on_submit():
        # Update session details with data from the form
        form.populate_obj(session)
        db.session.commit()
        flash('Session details updated successfully!', category='success')
        return redirect(url_for('views.display_sessions'))

    return render_template('edit_session.html', form=form, session=session, user=current_user)

@views.route('/delete-session/<int:id>', methods=['POST'])
@login_required
def delete_session(id):
    session = Session.query.get(id)
    if session:
        db.session.delete(session)
        db.session.commit()
        flash('Session deleted successfully!', category='success')
    return redirect(url_for('views.display_sessions'))

from flask import send_file


@views.route('/session_files', methods=['GET'])
@login_required
def session_files():
    user = current_user
    session_files_dir = user.folder_path  # Use the user's folder path

    files = []  # Create a list to store file paths and relative paths

    for root, _, filenames in os.walk(session_files_dir):
        for filename in filenames:
            file_path = os.path.join(root, filename)  # Get the full path of the file
            relative_path = os.path.relpath(file_path, session_files_dir)  # Relative path within the session folder
            files.append({'file_path': file_path, 'relative_path': relative_path})

    return render_template("session_files.html", files=files, user=user)

import os

@views.route('/download/<filename>', methods=['GET'])
@login_required
def download_file(filename):
    # Ensure that the user is authenticated and authorized to download files.
    user = current_user
    if current_user.is_authenticated:
        session_files_dir = user.folder_path
        file_path = os.path.join(session_files_dir, filename)

        if os.path.isfile(file_path):
            return send_file(file_path, as_attachment=True)
        else:
            flash('File not found', category='error')

    return redirect(url_for('views.session_files'))
