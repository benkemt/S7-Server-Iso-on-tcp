#include "WebUIContent.h"

std::string WebUIContent::getHTML() {
    return R"WEBUI(<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>S7 Server Web UI</title><style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background:#f5f5f5;color:#333}
.header{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;padding:20px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}
.header h1{font-size:24px;margin-bottom:5px}
.header .status{font-size:14px;opacity:0.9}
.status-dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#4ade80;margin-right:5px;animation:pulse 2s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.5}}
.container{display:flex;height:calc(100vh - 80px)}
.panel{flex:1;padding:20px;overflow-y:auto}
.panel-left{border-right:2px solid #e0e0e0;background:#fff}
.panel-right{background:#fafafa}
.section-title{font-size:18px;font-weight:600;margin-bottom:15px;color:#667eea}
.selector-group{margin-bottom:20px}
.selector-group label{display:block;margin-bottom:5px;font-weight:500;font-size:14px}
select{width:100%;padding:10px;border:2px solid #e0e0e0;border-radius:6px;font-size:14px;background:white;cursor:pointer}
select:focus{outline:none;border-color:#667eea}
.hex-display{background:#1e1e1e;color:#d4d4d4;padding:15px;border-radius:6px;font-family:'Courier New',monospace;font-size:13px;line-height:1.6;overflow-x:auto;white-space:pre;max-height:600px;overflow-y:auto}
.loading{text-align:center;padding:40px;color:#999}
.spinner{border:3px solid #f3f3f3;border-top:3px solid #667eea;border-radius:50%;width:40px;height:40px;animation:spin 1s linear infinite;margin:0 auto 15px}
@keyframes spin{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}
.tag-table{width:100%;border-collapse:collapse;background:white;border-radius:8px;overflow:hidden;box-shadow:0 2px 8px rgba(0,0,0,0.05)}
.tag-table th{background:#667eea;color:white;padding:12px;text-align:left;font-weight:600;font-size:13px}
.tag-table td{padding:10px 12px;border-bottom:1px solid #f0f0f0;font-size:13px}
.tag-table tr:hover{background:#f8f9ff}
.btn{padding:6px 12px;border:none;border-radius:4px;cursor:pointer;font-size:12px;font-weight:500}
.btn-edit{background:#667eea;color:white}
.btn-edit:hover{background:#5568d3}
.btn-resume{background:#4ade80;color:white;margin-left:5px}
.btn-resume:hover{background:#38c769}
.tag-value{font-weight:600;color:#667eea}
.badge{display:inline-block;padding:3px 8px;border-radius:12px;font-size:11px;font-weight:600}
.badge-manual{background:#fef3c7;color:#f59e0b}
.badge-auto{background:#d1fae5;color:#10b981}
.modal{display:none;position:fixed;z-index:1000;left:0;top:0;width:100%;height:100%;background:rgba(0,0,0,0.5)}
.modal-content{background:white;margin:10% auto;padding:30px;border-radius:12px;width:90%;max-width:500px;box-shadow:0 10px 40px rgba(0,0,0,0.2)}
.modal-header{font-size:20px;font-weight:600;margin-bottom:20px;color:#667eea}
.form-group{margin-bottom:15px}
.form-group label{display:block;margin-bottom:5px;font-weight:500;font-size:14px}
.form-group input{width:100%;padding:10px;border:2px solid #e0e0e0;border-radius:6px;font-size:14px}
.form-group input:focus{outline:none;border-color:#667eea}
.modal-buttons{display:flex;gap:10px;margin-top:20px}
.btn-primary{flex:1;padding:12px;background:#667eea;color:white;border:none;border-radius:6px;font-size:14px;font-weight:600;cursor:pointer}
.btn-primary:hover{background:#5568d3}
.btn-secondary{flex:1;padding:12px;background:#e0e0e0;color:#333;border:none;border-radius:6px;font-size:14px;font-weight:600;cursor:pointer}
.btn-secondary:hover{background:#d0d0d0}
.error-message{background:#fee2e2;color:#dc2626;padding:10px 15px;border-radius:6px;margin-top:10px;font-size:13px}
.refresh-info{margin-top:15px;padding:10px;background:#e0e7ff;border-radius:6px;font-size:13px;color:#667eea}
</style></head><body>
<div class="header"><h1>S7 Server Web UI</h1><div class="status"><span class="status-dot"></span><span id="statusText">Connected - Auto-refresh: 1s</span></div></div>
<div class="container">
<div class="panel panel-left"><div class="section-title">Memory Hex Viewer</div>
<div class="selector-group"><label>Select Memory Area:</label><select id="areaSelector">
<option value="">-- Select Area --</option><optgroup label="Data Blocks" id="dbGroup"><option value="loading">Loading...</option></optgroup>
<optgroup label="Other Areas"><option value="area-I">Input Area (I)</option><option value="area-Q">Output Area (Q)</option>
<option value="area-M">Merker/Flags (M)</option><option value="area-T">Timers (T)</option><option value="area-C">Counters (C)</option>
</optgroup></select></div><div id="hexContent"><div class="loading"><div class="spinner"></div><div>Select a memory area to view</div></div></div></div>
<div class="panel panel-right"><div class="section-title">Tag Values & Editor</div><div id="tagContent">
<div class="loading"><div class="spinner"></div><div>Loading tags...</div></div></div>
<div class="refresh-info">🔄 Tag values automatically update every 1 second<br>🔒 Manual edits disable auto-update for that tag</div></div></div>
<div id="editModal" class="modal"><div class="modal-content"><div class="modal-header">Edit Tag Value</div>
<div class="form-group"><label>Tag Address:</label><input type="text" id="modalAddress" readonly></div>
<div class="form-group"><label>Current Value:</label><input type="text" id="modalCurrentValue" readonly></div>
<div class="form-group"><label>New Value:</label><input type="number" id="modalNewValue" step="any"></div>
<div class="form-group"><label>Valid Range:</label><input type="text" id="modalRange" readonly></div>
<div id="modalError"></div><div class="modal-buttons">
<button class="btn-primary" onclick="saveTagValue()">Save</button>
<button class="btn-secondary" onclick="closeModal()">Cancel</button></div></div></div>
<script>
let currentEditTag=null,refreshInterval=null,hexRefreshInterval=null,currentHexSelection='';
async function loadDataBlocks(){try{const r=await fetch('/api/memory/dbs'),d=await r.json(),g=document.getElementById('dbGroup');g.innerHTML='';
if(d.dataBlocks&&d.dataBlocks.length>0){d.dataBlocks.forEach(n=>{const o=document.createElement('option');o.value='db-'+n;o.textContent='DB'+n;g.appendChild(o)})}
else{g.innerHTML='<option value="">No Data Blocks found</option>'}}catch(e){console.error('Error:',e);document.getElementById('dbGroup').innerHTML='<option value="">Error loading DBs</option>'}}
async function loadHexDump(s,showLoading=true){currentHexSelection=s;const h=document.getElementById('hexContent');
if(showLoading){h.innerHTML='<div class="loading"><div class="spinner"></div><div>Loading...</div></div>'}
try{let u;if(s.startsWith('db-')){u='/api/memory/db/'+s.substring(3)}else if(s.startsWith('area-')){u='/api/memory/area/'+s.substring(5)}else{return}
const r=await fetch(u),d=await r.json();if(d.error){h.innerHTML='<div class="error-message">'+d.error+'</div>';return}
h.innerHTML='<div class="hex-display">'+escapeHtml(d.hexDump)+'</div>'}catch(e){h.innerHTML='<div class="error-message">Error: '+e.message+'</div>'}}
async function refreshHexDump(){if(currentHexSelection){await loadHexDump(currentHexSelection,false)}}
async function loadTags(){try{const r=await fetch('/api/tags'),d=await r.json(),c=document.getElementById('tagContent');
if(!d.tags||d.tags.length===0){c.innerHTML='<div class="loading"><div>No tags configured</div></div>';return}
let h='<table class="tag-table"><thead><tr><th>Address</th><th>Type</th><th>Value</th><th>Range</th><th>Status</th><th>Actions</th></tr></thead><tbody>';
d.tags.forEach(t=>{const s=t.manualOverride?'<span class="badge badge-manual">🔒 Manual</span>':'<span class="badge badge-auto">🔄 Auto</span>';
h+='<tr><td><strong>'+escapeHtml(t.address)+'</strong></td><td>'+escapeHtml(t.dataType)+'</td><td class="tag-value">'+t.currentValue.toFixed(2)+'</td>';
h+='<td>'+t.minValue+' to '+t.maxValue+'</td><td>'+s+'</td><td><button class="btn btn-edit" onclick=\'editTag('+JSON.stringify(t)+')\'>Edit</button>';
if(t.manualOverride){h+='<button class="btn btn-resume" onclick=\'resumeAuto("'+escapeHtml(t.address)+'")\'>Resume Auto</button>'}
h+='</td></tr>'});h+='</tbody></table>';c.innerHTML=h}catch(e){console.error('Error:',e);
document.getElementById('tagContent').innerHTML='<div class="error-message">Error: '+e.message+'</div>'}}
function editTag(t){currentEditTag=t;document.getElementById('modalAddress').value=t.address;
document.getElementById('modalCurrentValue').value=t.currentValue.toFixed(4);
document.getElementById('modalNewValue').value=t.currentValue.toFixed(4);
document.getElementById('modalRange').value=t.minValue+' to '+t.maxValue;
document.getElementById('modalError').innerHTML='';document.getElementById('editModal').style.display='block';
document.getElementById('modalNewValue').focus()}
function closeModal(){document.getElementById('editModal').style.display='none';currentEditTag=null}
async function saveTagValue(){const v=parseFloat(document.getElementById('modalNewValue').value),e=document.getElementById('modalError');
if(isNaN(v)){e.innerHTML='<div class="error-message">Invalid number</div>';return}
if(v<currentEditTag.minValue||v>currentEditTag.maxValue){e.innerHTML='<div class="error-message">Value must be between '+currentEditTag.minValue+' and '+currentEditTag.maxValue+'</div>';return}
try{const r=await fetch('/api/tags/update',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({address:currentEditTag.address,value:v})}),
d=await r.json();if(d.error){e.innerHTML='<div class="error-message">'+d.error+'</div>';return}closeModal();loadTags()}
catch(err){e.innerHTML='<div class="error-message">Error: '+err.message+'</div>'}}
async function resumeAuto(a){try{const r=await fetch('/api/tags/resume-auto',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({address:a})}),
d=await r.json();if(d.success){loadTags()}}catch(e){console.error('Error:',e)}}
function escapeHtml(t){const d=document.createElement('div');d.textContent=t;return d.innerHTML}
document.getElementById('areaSelector').addEventListener('change',function(e){if(e.target.value){loadHexDump(e.target.value);
if(hexRefreshInterval){clearInterval(hexRefreshInterval)}hexRefreshInterval=setInterval(refreshHexDump,1000)}else{if(hexRefreshInterval){clearInterval(hexRefreshInterval);hexRefreshInterval=null}currentHexSelection=''}});
window.onclick=function(e){const m=document.getElementById('editModal');if(e.target===m){closeModal()}};
window.onload=function(){loadDataBlocks();loadTags();refreshInterval=setInterval(loadTags,1000)};
</script></body></html>)WEBUI";
}
