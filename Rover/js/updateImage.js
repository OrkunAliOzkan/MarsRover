const reloadImg = url =>
  fetch(url, { cache: 'reload', mode: 'no-cors' })
  .then(() => document.body.querySelectorAll(`img[src='${url}']`)
              .forEach(img => img.src = url));

function updateImage()
{
    reloadImg("../map/map.jpg");
    console.log("This has run?");
    setTimeout(updateImage, 1000);
}

updateImage();