var imageData=[];


var getFileBlob = function (url, cb) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", url);
        xhr.responseType = "blob";
        xhr.addEventListener('load', function() {
            cb(xhr.response);
        });
        xhr.send();
};

var blobToFile = function (blob, name) {
        blob.lastModifiedDate = new Date();
        blob.name = name;
        return blob;
};

var getFileObject = function(filePathOrUrl, cb) {
       getFileBlob(filePathOrUrl, function (blob) {
          cb(blobToFile(blob, 'test.jpg'));
       });
};
getFileObject('\heightmap.bmp', function (fileObject) {
     console.log(fileObject);
	 var reader = new FileReader();  reader.addEventListener("load",
                       processimage, false);  reader.readAsArrayBuffer(fileObject);

}); 
function processimage(e) {
 var buffer = e.target.result;
 var bitmap = getBMP(buffer); 
 imageData = convertToImageData(bitmap);  
}

function getBMP(buffer) {
 var datav = new DataView(buffer);
 var bitmap = {};
bitmap.fileheader = {}; bitmap.fileheader.bfType =
                datav.getUint16(0, true);
bitmap.fileheader.bfSize =
                datav.getUint32(2, true);
bitmap.fileheader.bfReserved1 =
                datav.getUint16(6, true);
bitmap.fileheader.bfReserved2 =
                datav.getUint16(8, true);
bitmap.fileheader.bfOffBits =
                datav.getUint32(10, true);
bitmap.infoheader = {};
bitmap.infoheader.biSize =
                datav.getUint32(14, true);
bitmap.infoheader.biWidth =
                datav.getUint32(18, true);
bitmap.infoheader.biHeight =
                datav.getUint32(22, true);
bitmap.infoheader.biPlanes =
                datav.getUint16(26, true);
bitmap.infoheader.biBitCount =
                datav.getUint16(28, true);
bitmap.infoheader.biCompression =
                datav.getUint32(30, true);
bitmap.infoheader.biSizeImage =
                datav.getUint32(34, true);
bitmap.infoheader.biXPelsPerMeter =
                datav.getUint32(38, true);
bitmap.infoheader.biYPelsPerMeter =
                datav.getUint32(42, true);
bitmap.infoheader.biClrUsed =
                datav.getUint32(46, true);
bitmap.infoheader.biClrImportant =
                datav.getUint32(50, true);
var start = bitmap.fileheader.bfOffBits;  bitmap.stride =
  Math.floor((bitmap.infoheader.biBitCount
    *bitmap.infoheader.biWidth +
                            31) / 32) * 4;
 bitmap.pixels =
         new Uint8Array(buffer, start);
 return bitmap;
}		

function convertToImageData(bitmap) {
 canvas = document.createElement("canvas");
 var ctx = canvas.getContext("2d");
 var Width = bitmap.infoheader.biWidth;
 var Height = bitmap.infoheader.biHeight;
 var imageData = ctx.createImageData(
                           Width, Height);
						   var data = imageData.data;
var bmpdata = bitmap.pixels;
var stride = bitmap.stride;
for (var y = 0; y < Height; ++y) {
 for (var x = 0; x < Width; ++x) {
  var index1 = (x+Width*(Height-y))*4;
  var index2 = x * 3 + stride * y;
  data[index1] = bmpdata[index2 + 2];
  data[index1 + 1] = bmpdata[index2 + 1];
  data[index1 + 2] = bmpdata[index2];
  data[index1 + 3] = 255;
 }
}return imageData;
}