<!DOCTYPE html>
<html>

<head>
<style>
table.list {
	border-collapse: collapse;
}
.list {
    border: 1px solid black;
}

.equ {
	vertical-align: top;
	border: 0px;
}

.swap img {
	width: auto;
	height: auto;
}
.swap img.org{display:none}
.swap:hover img.overlay{display:none}
.swap:hover img.org{display:inline-block}
</style>
</head>

<body>

<h1>
{{ key }}
</h1>

<h2>Overall</h2>
<table class="equ">
	<tr class="equ">
		<td class="equ">TP:</td>
		<td class="equ">{{ per_case_result["tp"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">FP:</td>
		<td class="equ">{{ per_case_result["fp"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">FN:</td>
		<td class="equ">{{ per_case_result["fn"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Sensitivity:</td>
		<td class="equ">{{ per_case_result["sensitivity"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Normalized FP:</td>
		<td class="equ">{{ per_case_result["normalized_fp"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Overlap ratio:</td>
		<td class="equ">{{ per_case_result["overlap_ratio"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Slice spacing:</td>
		<td class="equ">{{ overall["spacing"][2] }}</td>
	</tr>
</table>

<h2>Missed (FN)</h2>
{% if fn -%}
<table class="list">
	<tr class="list">
		<th class="list">Axial,Coronal,Sagittal</th>
		<th class="list">Reference ID</th>
		<th class="list">Diameter</th>
		<th class="list">Perpendicular Diameter</th>
	</tr>
{% for info in fn -%}
	<tr class="list">
		<td class="list">
			<a class="swap">
			<img class="overlay" src="{{ to_html(info["a_ovr"]) }}"/>
			<img class="org" src="{{ to_html(info["a_org"]) }}"/>
			
			<img class="overlay" src="{{ to_html(info["c_ovr"]) }}"/>
			<img class="org" src="{{ to_html(info["c_org"]) }}"/>
			
			<img class="overlay" src="{{ to_html(info["s_ovr"]) }}"/>
			<img class="org" src="{{ to_html(info["s_org"]) }}"/>
			</a>
		</td>
		<td class="list">{{info["truth_id"]}}</td>
		<td class="list">{{info["truth_diameter"]}}</td>
		<td class="list">{{info["truth_perp_diameter"]}}</td>		
	</tr>
{%- endfor %}
</table>
{%- else -%}
None
{%- endif %}

<h2>True Positives</h2>
{% if tp -%}
<table class="list">
	<tr class="list">
		<th class="list">Axial,Coronal,Sagittal</th>
		<th class="list">Reference ID</th>
		<th class="list">Diameter</th>
		<th class="list">Perpendicular Diameter</th>
		<th class="list">CAD ID</th>
		<th class="list">CAD Diameter</th>
		<th class="list">CAD Perpendicular Diameter</th>
	</tr>
{% for info in tp -%}
	<tr class="list">
		<td class="list">
			<a class="swap">
			<img class="overlay" src="{{ to_html(info["a_ovr"]) }}"/>
			<img class="org" src="{{ to_html(info["a_org"]) }}"/>
			
			<img class="overlay" src="{{ to_html(info["c_ovr"]) }}"/>
			<img class="org" src="{{ to_html(info["c_org"]) }}"/>
			
			<img class="overlay" src="{{ to_html(info["s_ovr"]) }}"/>
			<img class="org" src="{{ to_html(info["s_org"]) }}"/>
			</a>
		</td>
		<td class="list">{{info["truth_id"]}}</td>
		<td class="list">{{info["truth_diameter"]}}</td>
		<td class="list">{{info["truth_perp_diameter"]}}</td>
		<td class="list">{{info["id"]}}</td>
		<td class="list">{{info["diameter"]}}</td>
		<td class="list">{{info["perp_diameter"]}}</td>			
	</tr>
{%- endfor %}
</table>
{%- else -%}
None
{%- endif %}

<h2>False Positives</h2>
{% if fp -%}
<table class="list">
	<tr class="list">
		<th class="list">Axial,Coronal,Sagittal</th>
		<th class="list">CAD ID</th>
		<th class="list">CAD Diameter</th>
		<th class="list">CAD Perpendicular Diameter</th>
	</tr>
{% for info in fp -%}
	<tr class="list">
		<td class="list">
			<a class="swap">
			<img class="overlay" src="{{ to_html(info["a_ovr"]) }}"/>
			<img class="org" src="{{ to_html(info["a_org"]) }}"/>
			
			<img class="overlay" src="{{ to_html(info["c_ovr"]) }}"/>
			<img class="org" src="{{ to_html(info["c_org"]) }}"/>
			
			<img class="overlay" src="{{ to_html(info["s_ovr"]) }}"/>
			<img class="org" src="{{ to_html(info["s_org"]) }}"/>
			</a>
		</td>
		<td class="list">{{info["id"]}}</td>
		<td class="list">{{info["diameter"]}}</td>
		<td class="list">{{info["perp_diameter"]}}</td>			
	</tr>
{%- endfor %}
</table>
{%- else -%}
None
{%- endif %}


</body>

</html>