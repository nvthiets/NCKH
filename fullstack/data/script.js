document.addEventListener("DOMContentLoaded", () => {
    fetchData(); // Lấy dữ liệu ban đầu
    setInterval(fetchData, 5000); // Cập nhật dữ liệu mỗi 5 giây

    // Thêm sự kiện tìm kiếm
    document.getElementById("searchInput").addEventListener("input", (event) => {
        const searchTerm = event.target.value.trim().toLowerCase(); // Lấy từ khóa tìm kiếm
        console.log("Từ khóa tìm kiếm:", searchTerm); // Debug từ khóa
        filterTable(searchTerm); // Lọc bảng
    });
});

// Lấy dữ liệu từ ESP32
function fetchData() {
    fetch("/data")
        .then(response => response.json())
        .then(data => {
            console.log("Dữ liệu từ ESP32:", data); // Debug dữ liệu
            updateTable(data); // Cập nhật bảng với dữ liệu mới
        })
        .catch(error => console.error("Lỗi lấy dữ liệu từ ESP32:", error));
}

// Cập nhật bảng dữ liệu
function updateTable(productData) {
    let table = document.getElementById("productTable");
    table.innerHTML = ""; // Xóa dữ liệu cũ
    let totalItems = 0;

    // Thêm dữ liệu mới vào bảng
    productData.forEach(product => {
        let row = `<tr>
            <td>${product.id}</td>
            <td>${product.name}</td>
            <td>${product.quantity}</td>
            <td>${product.date}</td>
            <td>${product.time}</td>
        </tr>`;
        table.innerHTML += row; // Thêm hàng mới
        totalItems++; // Tăng tổng số lượng hàng
    });

    // Cập nhật thống kê
    document.getElementById("totalItems").innerText = totalItems;
    document.getElementById("lowStock").innerText = 0; // Không sử dụng
    document.getElementById("outOfStock").innerText = 0; // Không sử dụng
}

// Lọc bảng dữ liệu theo từ khóa tìm kiếm
function filterTable(searchTerm) {
    let table = document.getElementById("productTable");
    let rows = table.getElementsByTagName("tr");

    console.log("Tổng số hàng:", rows.length); // Debug số lượng hàng

    for (let i = 1; i < rows.length; i++) { // Bỏ qua hàng tiêu đề (index 0)
        let row = rows[i];
        let name = row.getElementsByTagName("td")[1].textContent.toLowerCase(); // Lấy tên hàng
        console.log("Tên hàng:", name); // Debug tên hàng

        // Hiển thị hoặc ẩn hàng dựa trên từ khóa tìm kiếm
        if (name.includes(searchTerm)) {
            row.style.display = ""; // Hiển thị hàng phù hợp
            console.log("Hiển thị hàng:", name); // Debug hàng được hiển thị
        } else {
            row.style.display = "none"; // Ẩn hàng không phù hợp
            console.log("Ẩn hàng:", name); // Debug hàng bị ẩn
        }
    }
}