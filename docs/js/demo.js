
const R={USD:1,TWD:32.4,JPY:154.2,EUR:.92,GBP:.79,CNY:7.24,KRW:1382,HKD:7.8,KWD:.307};
const N={USD:'美元',TWD:'新台幣',JPY:'日圓',EUR:'歐元',GBP:'英鎊',CNY:'人民幣',KRW:'韓元',HKD:'港幣',KWD:'科威特第納爾'};
const D={JPY:0,KRW:0,KWD:3};
cl.innerHTML=Object.keys(R).map(k=>`<option value="${k}">${N[k]}</option>`).join('');
const pick=v=>{v=v.trim().toUpperCase();if(R[v])return v;return Object.keys(N).find(k=>N[k].includes(v)||k.includes(v))};
const fm=(n,c)=>n.toLocaleString('en-US',{minimumFractionDigits:D[c]??2,maximumFractionDigits:D[c]??2});
function go(){
 const f=pick(fs.value),t=pick(ts.value),a=+amt.value||0;
 if(!f||!t){res.textContent='請選擇有效的幣別';rt.textContent='';return}
 const rate=R[t]/R[f];
 res.textContent=`${fm(a,f)} ${f} = ${fm(a*rate,t)} ${t}`;
 rt.textContent=f===t?'來源與目標相同,匯率為 1,不會發出網路請求。':`1 ${f} = ${rate.toFixed(D[t]===0?2:4)} ${t}`;
}
[fs,ts,amt].forEach(e=>e.addEventListener('input',go));
sw.onclick=()=>{[fs.value,ts.value]=[ts.value,fs.value];go()};
rf.onclick=()=>{ts2.textContent='已更新 '+new Date().toLocaleTimeString();toast('已重新取得匯率(示意)');go()};
go();
