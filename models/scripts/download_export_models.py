from ultralytics import YOLO

# Load a model
model_n = YOLO("yolo11n-seg.pt")  # load an official model
model_x = YOLO("yolo11x-seg.pt")  # load an official model

# Export the model
model_n.export(format="onnx")
model_n.export(format="ncnn")

model_x.export(format="onnx")
model_x.export(format="ncnn")

