# Get Screen Pixel CPP

A C++ program to pick the specific color of a pixel on screen

## Installation

If you are running a Linux or Mac OS machine, you will have to compile the C++ code or run it from your own editor. 
```bash
# For Linux
sudo apt update
sudo apt install build-essential libx11-dev
g++ -std=c++11 your_file.cpp -o your_program -lX11 -lpthread

# For Mac OS
xcode-select --install
g++ -std=c++11 your_file.cpp -o your_program
```
If you are running windows you can simply run the color_picker.exe file

## Usage
If you are running on a windows machine, you can run the color_picker.exe file which will start the program automatically.

If you are using another operating system however, you may have to compile the code yourself or you can compile it within your IDE. Whatever you do is up to you!

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change. I realize this program may not be the best so any changes would be appreaciated.

Please make sure to update tests as appropriate.

## License

[MIT](https://choosealicense.com/licenses/mit/)