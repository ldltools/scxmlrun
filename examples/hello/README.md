# hello

## [hello.scxml](hello.scxml)

```xml
<scxml xmlns="http://www.w3.org/2005/07/scxml" version="1.0">
  <final id="hello">
    <onentry><log label="hello" expr="world!"/></onentry>
  </final>
</scxml>
```

## scxmlrun

`scxmlrun hello.scxml`
