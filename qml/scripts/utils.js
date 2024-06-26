.pragma library

function calculateScale(width, height, cellSize) {
    var widthScale = (cellSize * 1.0) / width
    var heightScale = (cellSize * 1.0) / height
    var scale = 0

    if (widthScale <= heightScale) {
        scale = widthScale;
    } else if (heightScale < widthScale) {
        scale = heightScale;
    }
    return scale;
}

function shuffle(model){
    var currentIndex = model.count, temporaryValue, randomIndex;

    // While there remain elements to shuffle...
    while (0 !== currentIndex) {
        // Pick a remaining element...
        randomIndex = Math.floor(Math.random() * currentIndex)
        currentIndex -= 1
        // And swap it with the current element.
        // the dictionaries maintain their reference so a copy should be made
        // https://stackoverflow.com/a/36645492/6622587
        temporaryValue = JSON.parse(JSON.stringify(model.get(currentIndex, "fileURL")))
        model.set(currentIndex, model.get(randomIndex, "fileURL"))
        model.set(randomIndex, temporaryValue);
    }
    return model;
}
