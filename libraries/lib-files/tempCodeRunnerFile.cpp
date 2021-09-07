        ShowMessageBox(
            XO("Directory %s does not have write permissions").Format(path),
            MessageBoxOptions{}
                .Caption(XO("Error"))
                .IconStyle(Icon::Error)
                .ButtonStyle(Button::Ok)
        );
    }
