from flask_wtf import FlaskForm
from wtforms import IntegerField, TextAreaField
from wtforms.validators import InputRequired

class PatientForm(FlaskForm):
    id = IntegerField('Patient ID', validators=[InputRequired()])
    behaviors = TextAreaField('Behaviors', validators=[InputRequired()])


from flask_wtf import FlaskForm
from wtforms import IntegerField, TextAreaField
from wtforms.validators import InputRequired

class EditPatientForm(FlaskForm):
    id = IntegerField('Patient ID', validators=[InputRequired()])
    behaviors = TextAreaField('Behaviors', validators=[InputRequired()])

from flask_wtf import FlaskForm
from wtforms import StringField, IntegerField

from flask_wtf import FlaskForm
from wtforms import StringField, TextAreaField, SelectField
from wtforms.validators import InputRequired

class SessionForm(FlaskForm):
    patient_id = SelectField('Patient ID', coerce=int, validators=[InputRequired()])
    session_id = StringField('Session ID', validators=[InputRequired()])
    additional_information = TextAreaField('Additional Information')

from flask_wtf import FlaskForm
from wtforms import StringField, TextAreaField
from wtforms.validators import DataRequired

class EditSessionForm(FlaskForm):
    session_id = StringField('Session ID', validators=[DataRequired()])
    additional_information = TextAreaField('Additional Information')


