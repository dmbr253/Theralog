#APP ENTRY POINT

# website is a python package. When you import the name of the folder (website) it will by default run all the stuff in the .py files
# This means we can import anything defined in the .py files for example, (create_app) defined in (_init_.py)
from website import create_app

app = create_app()

# Only if we run main.py (not import main.py) 
# The reason you need this is if main.py was imported in a different file, it would run the webserver.
if __name__ == '__main__':
    # Will run our Flask application and create a running website
    # Debug = true means that anytime we make a change to our python code it will automatically rerun the webserver
    
    #I changed this line of code because it was only listening to requests from the local host
    #app.run(debug=True) 

    #With this code, the server listens to outside requests as well
    app.run(host="0.0.0.0", port=5000, debug=True)
