const express = require("express");
const mongoose = require("mongoose");
const cors = require("cors");
require("dotenv").config();

const app = express();
const port = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());

mongoose.connect(process.env.MONGODB_URL, {useNewUrlParser: true, useUnifiedTopology: true})
	.then(() => console.log("Connected to MongoDB"))
	.catch(err => console.error("MongoDB Connection Error: ", err));


const sensorDataSchema = new mongoose.Schema({
	sensorType: String,
	value: Number,
	timestamp: {type: Date, default: Date.now}
});

const SensorData = mongoose.model("SensorData", sensorDataSchema);

app.post("/api/sensor-data", async (req, res) => {
	try{
    	const {sensorType, value} = req.body;
    	const newData = new SensorData({sensorType, value});
    	await newData.save();
    	res.status(201).json({message: "Data Was Saved to MongoDB"});
	}
	catch (error){
    	res.status(500).json({error: error.message});
	}
});

app.listen(port, "0.0.0.0", () => {
	console.log(`The Server Is Running On Port: ${port}`);
});
