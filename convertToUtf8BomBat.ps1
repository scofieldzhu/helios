
# 设置控制台输出编码为UTF-8  
# [Console]::OutputEncoding = [System.Text.Encoding]::UTF8  

$folderPath = "E:\Code\GIT\visual\source"  

# 获取文件夹及其子文件夹下的所有 .h, .cpp 和 .hpp 文件  
$files = Get-ChildItem -Path $folderPath -Recurse -File | Where-Object { $_.Extension -in ".h", ".cpp", ".hpp", ".hxx" }  

foreach ($file in $files) {  
    # 读取文件内容，指定编码为默认编码（假设原始文件是UTF-8无BOM或其他编码）  
    $content = Get-Content -Path $file.FullName -Raw -Encoding Default  

    # 将内容写回文件，使用UTF-8 BOM编码  
    [System.IO.File]::WriteAllText($file.FullName, $content, [System.Text.Encoding]::UTF8)  

    # 输出处理的文件名  
    Write-Host "已转换文件: $($file.FullName)"  
}  

Write-Host "所有文件已转换为UTF-8 BOM编码"  