from flask import Flask, jsonify, render_template
from pathlib import Path
import pandas as pd

app = Flask(__name__, template_folder = "../templates")

CSV = Path("../Resource_Monitor.csv")

@app.route("/")
def dashboard():
    return render_template("dashboard.html")

@app.route("/api/dados")
def api_dados():
    if not CSV.exists():
        return jsonify([])
    
    try:
        dataframe = pd.read_csv(CSV)
    except:
        return jsonify([])
    
    dataframe = dataframe.tail(100)

    dados = dataframe.to_dict(orient = "records")
    return jsonify(dados)

if __name__ == "__main__":
    app.run(debug = True)
    