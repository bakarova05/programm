#include <opencv2/opencv.hpp>  
#include <iostream>  
using namespace cv;  
using namespace std;  
// Глобальные переменные для параметров обработки
int brightness = 50; // начальное значение яркости 
int contrast = 50; // начальное значение контраста 
int saturation = 50; // начальное значение насыщенности 
double shadowValue = 0.5; // значение теней
double highlightValue = 0.5; // значение света
double vignetteStrength = 0.05; // сила виньетирования

// Функция для изменения яркости  
Mat changeBrightness(const Mat& image, int value) {  
    Mat newImage;  
    image.convertTo(newImage, -1, 1, value - 50); // value - значение яркости  
    return newImage;  
}  
  
// Функция для изменения контраста  
Mat changeContrast(const Mat& image, double alpha) {  
    Mat newImage;  
    image.convertTo(newImage, -1, alpha / 50.0, 0); // alpha - коэффициент контраста  
    return newImage;  
}  
  
// Функция для изменения насыщенности  
Mat changeSaturation(const Mat& image, double saturationFactor) {  
    Mat hsvImage;  
    cvtColor(image, hsvImage, COLOR_BGR2HSV);  
    for (int y = 0; y < hsvImage.rows; y++) {  
        for (int x = 0; x < hsvImage.cols; x++) {  
            Vec3b& pixel = hsvImage.at<Vec3b>(y, x);  
            pixel[1] = saturate_cast<uchar>(pixel[1] * saturationFactor / 50.0); // Изменение насыщенности  
        }  
    }  
    Mat newImage;  
    cvtColor(hsvImage, newImage, COLOR_HSV2BGR);  
    return newImage;  
}  
  
// Функция для изменения теней и света  
Mat changeShadowsAndHighlights(const Mat& image, double shadowValue, double highlightValue) {  
    Mat newImage;  
    image.convertTo(newImage, CV_32F);  
    newImage = newImage * (1 + highlightValue); // Увеличение света  
    newImage = newImage - (newImage * shadowValue); // Уменьшение теней  
    newImage.convertTo(newImage, CV_8U);  
    return newImage;  
}  
  
// Функция для виньетирования  
Mat applyVignette(const Mat& image, double strength) {  
    Mat newImage = image.clone();  
    int rows = image.rows;  
    int cols = image.cols;  
  
    for (int y = 0; y < rows; y++) {  
        for (int x = 0; x < cols; x++) {  
            double distance = sqrt(pow(x - cols / 2.0, 2) + pow(y - rows / 2.0, 2));  
            double vignette = exp(-strength * distance / (rows / 2.0));  
            newImage.at<Vec3b>(y, x)[0] *= vignette; // B  
            newImage.at<Vec3b>(y, x)[1] *= vignette; // G  
            newImage.at<Vec3b>(y, x)[2] *= vignette; // R  
        }  
    }  
      
    return newImage;  
}  

// Функция для обновления изображения
void updateImage(int, void*) {
    Mat image = imread("C:/Users/bakar/source/repos/Redactor/image.jpg");
    
    if (image.empty()) {
        cout << "Could not open or find the image!" << endl;
        return;
    }
    
    // Применяем все изменения к исходному изображению
    Mat brightened = changeBrightness(image, brightness);
    Mat contrasted = changeContrast(brightened, contrast);
    Mat saturated = changeSaturation(contrasted, saturation);
    Mat shadowsHighlights = changeShadowsAndHighlights(saturated, shadowValue, highlightValue);
    Mat vignetted = applyVignette(shadowsHighlights, vignetteStrength);

    // Отображаем только одно итоговое изображение
    imshow("Processed Image", vignetted);
}

int main() {  

    // Создание окна для отображения итогового изображения
    namedWindow("Processed Image");

    // Создание ползунков
    createTrackbar("Brightness", "Processed Image", &brightness, 100, updateImage);
    createTrackbar("Contrast", "Processed Image", &contrast, 100, updateImage);
    createTrackbar("Saturation", "Processed Image", &saturation, 100, updateImage);
    createTrackbar("Shadow Value", "Processed Image", reinterpret_cast<int*>(&shadowValue), 100, updateImage);
    createTrackbar("Highlight Value", "Processed Image", reinterpret_cast<int*>(&highlightValue), 100, updateImage);
    createTrackbar("Vignette Strength", "Processed Image", reinterpret_cast<int*>(&vignetteStrength), 100, updateImage);

    // Инициализация отображения
    updateImage(0, nullptr);

    waitKey(0);  
    return 0;  
}
