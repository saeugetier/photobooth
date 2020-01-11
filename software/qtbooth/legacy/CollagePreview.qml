import QtQuick 2.0

Grid {
    rows: 2
    columns: 2
    spacing: width * 0.05

    states:
    {
        State
        {
            names: "1photos"
        }
        State
        {
            names: "2photos"
        }
        State
        {
            names: "3photos"
        }
        State
        {
            names: "4photos"
        }
    }

    PhotoProxy
    {
        id: proxy1

    }
    PhotoProxy
    {
        id: proxy2
    }
    PhotoProxy
    {
        id: proxy3
    }
    PhotoProxy
    {
        id: proxy4
    }
}
